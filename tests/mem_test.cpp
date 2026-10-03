/*********************************************
 * 工程1 検証: mnode（refcnt 付き）+ mallocator + operator new 差し替え
 *
 * 観測対象:
 *   1. 整列・ヘッダフィールド（provided_size / size / state / refcnt=0 開始）
 *   2. refcnt プロトコル（incr/decr/到達判定/飽和ガード/0 のときの不作為）
 *   3. dealloc の冪等性・前方併合・share==0 での全リセット
 *   4. 再利用パス（分割 / 非分割）と非分割時のノード走査整合性
 *      （devarm 退行の再発防止: 非分割で size を上書きしない）
 *   5. 実 ABI（AppleClang/AArch64 Itanium）での cookie 判別:
 *      非トリビアル型の new[] / スカラ new / トリビアル配列の三態で
 *      mnode_get / qty / decr_or_delete(delete か delete[] か) が正しいこと
 *   5b. cookie 8 バイト（x86-64）の幾何での判別と、ずれた候補を読まないこと
 *   6. 静的初期化中の new（遅延 init）の安全性
 *
 * この実行ファイルは mem_global_new をリンクするため、std コンテナ等の
 * 暗黙確保を持ち込まない（assert / printf のみ使用）。
 * assert 式に副作用を入れない（NDEBUG ビルドで消えるため）。
 *********************************************/
#include <cassert>
#include <cstdio>
#include <cstdint>
#include "mem/mnode.h"
#include "mem/mallocator.h"

/* 非トリビアル destructor を持つ型 -> new[] で cookie が付く */
struct tracked_ {
	long v;
	tracked_() : v(7) {}
	~tracked_(){ ++dtor_count; }
	static int dtor_count;
};
int tracked_::dtor_count = 0;

/* 静的初期化中の new: mallocator の遅延 init 経路を踏む */
static char* g_early = new char[8];

static_assert(sizeof(mnode_) == 32, "host layout: 8+8+4 -> pad to 32 (align 16)");
static_assert(mnode_new_header<char>() == 0, "trivial type: no cookie");
static_assert(mnode_new_header<tracked_>() != 0, "non-trivial: cookie (幅は ABI 依存)");

int main(){
	/* --- 1. 基本フィールド --- */
	unsigned char* a = mallocator.alloc(10);
	assert(((uintptr_t)a % MALLOCATOR_ALIGN) == 0);
	assert(mallocator.is_memory(a));
	MNODE_* ha = mnode_header(a);
	assert(ha->provided_size == 10);
	assert(ha->size == malloc_align_up(10));
	assert(ha->state & MALLOCATOR::USED);
	assert(ha->refcnt() == 0);                 /* 原典準拠: alloc 直後 refcnt=0 */
	assert(mnode_size(a) == 10);
	assert(mnode_qty(a, (size_)2) == 5);
	assert(mnode_qty(a, (size_)0) == ERR_N(MNODE_GET_NUM_DIVIDE_ZERO));
	assert(mnode_size((const void*)nullptr) == 0);   /* NUL_ */

	/* --- 2. refcnt プロトコル --- */
	assert(mnode_refcnt(a) == 0);
	{ bool r = mnode_refcnt_incr(a); assert(r); }
	{ bool r = mnode_refcnt_incr(a); assert(r); }
	assert(mnode_refcnt(a) == 2);
	{ bool r = mnode_refcnt_decr(a); assert(!r); }   /* 2->1 未到達 */
	{ bool r = mnode_refcnt_decr(a); assert(r);  }   /* 1->0 到達 */
	{ bool r = mnode_refcnt_decr(a); assert(!r); }   /* 0 のまま(NUL_)、破壊なし */
	assert(mnode_refcnt(a) == 0);
	for (int i = 0; i < 255; ++i){ bool r = mnode_refcnt_incr(a); assert(r); }
	assert(mnode_refcnt(a) == 255);
	{ bool r = mnode_refcnt_incr(a); assert(!r); }   /* 飽和: 拒否 */
	assert(mnode_refcnt(a) == 255);
	assert(ha->state & MALLOCATOR::USED);            /* 隣接ビット無傷 */
	mallocator.dealloc(a);

	/* --- 3-4. 再利用・併合・冪等・リセット --- */
	unsigned char* p[8];
	for (int i = 0; i < 8; ++i){
		p[i] = mallocator.alloc(1000);
		assert(((uintptr_t)p[i] % MALLOCATOR_ALIGN) == 0);
	}
	/* a の解放で share!=0 のまま。逆順 free で p[0..6] が一つの穴に併合される */
	for (int i = 6; i >= 0; --i) mallocator.dealloc(p[i]);
	{   /* 併合の確認: p[0] のノードが p[0..6] の全スパンを覆う */
		size_ node_sz = malloc_align_up(1000) + sizeof(MNODE_);
		assert(mnode_header(p[0])->size == 7 * node_sz - sizeof(MNODE_));
	}
	mallocator.dealloc(p[3]);                        /* 冪等: 穴の内部を再 free しても不変 */
	{
		size_ node_sz = malloc_align_up(1000) + sizeof(MNODE_);
		assert(mnode_header(p[0])->size == 7 * node_sz - sizeof(MNODE_));
	}

	/* 分割 reuse: 先頭の穴から 512 を切り出す */
	unsigned char* q1 = mallocator.alloc(500);
	assert(q1 == p[0]);                              /* 穴の先頭が再利用される */
	assert(mnode_header(q1)->size == malloc_align_up(500));
	assert(mnode_header(q1)->provided_size == 500);

	/* 非分割 reuse: q1 を free し（512 の穴）、490(->496) を要求。
	   512 <= 496+32+16 なので分割されず、node size は 512 のまま残る。
	   これが崩れるとノード走査が破綻する（devarm 退行の再発防止点） */
	unsigned char* q2 = mallocator.alloc(1000);      /* 残りの穴の先頭 */
	assert(q2 == q1 + malloc_align_up(500) + sizeof(MNODE_));
	mallocator.dealloc(q1);
	unsigned char* q3 = mallocator.alloc(490);
	assert(q3 == q1);
	assert(mnode_header(q3)->size == 512);           /* 上書きされない（原典準拠） */
	assert(mnode_header(q3)->provided_size == 490);
	/* 走査整合性: q3 ノードの直後がちょうど q2 のヘッダ */
	assert((unsigned char*)mnode_header(q3) + sizeof(MNODE_) + mnode_header(q3)->size
	       == (unsigned char*)mnode_header(q2));

	/* --- 5. 実 ABI での cookie 判別（global new/delete 差し替え経由） --- */
	tracked_::dtor_count = 0;
	tracked_* s = new tracked_;                      /* スカラ: cookie 無し */
	assert(mallocator.is_memory(s));
	assert(mnode_get(s) == mnode_header(s));
	assert(!is_mnode_class_array(mnode_get(s)));
	{ bool r = mnode_refcnt_incr(s); assert(r); }
	mnode_refcnt_decr_or_delete(s);                  /* 1->0 で delete */
	assert(tracked_::dtor_count == 1);

	tracked_* arr = new tracked_[5];                 /* 非トリビアル配列: cookie 有り */
	assert(mallocator.is_memory(arr));
	assert(is_mnode_class_array(mnode_get(arr)));
	assert(mnode_qty(arr) == 5);                     /* cookie の要素数を全幅で読む */
	assert(mnode_qty_class(arr) == 5);
	{ bool r = mnode_refcnt_incr(arr); assert(r); }
	mnode_refcnt_decr_or_delete(arr);                /* 1->0 で delete[] -> dtor x5 */
	assert(tracked_::dtor_count == 6);

	tracked_* t2 = new tracked_;                     /* refcnt=0 は非管理: 何もしない */
	mnode_refcnt_decr_or_delete(t2);
	assert(tracked_::dtor_count == 6);
	delete t2;
	assert(tracked_::dtor_count == 7);

	char* ca = new char[100];                        /* トリビアル配列: cookie 無し + CLASS_ARRAY */
	assert(mnode_get(ca) == mnode_header(ca));
	assert(is_mnode_class_array(mnode_get(ca)));
	assert(mnode_qty(ca) == 100);                    /* provided_size / sizeof(char) */
	assert(mnode_qty_class(ca) == 0);                /* cookie 無し -> 0（原典どおり） */
	{ bool r = mnode_refcnt_incr(ca); assert(r); }
	mnode_refcnt_decr_or_delete(ca);                 /* flag により delete[] が選ばれる */

	/* --- 5b. cookie が整列幅より小さい ABI の幾何（どの ABI でも検査する） ---
	   x86-64 の new[] cookie は 8 バイトで、direct 候補が 16 バイト境界から外れる。
	   AArch64 の cookie は 16 バイトなので 5. ではこの形を通らない。ヘッダを
	   手で並べ、判別が正しいことと、ずれた候補を mnode_* として読まないこと
	   （UBSan ビルドで alignment 違反が出ないこと）を確かめる。 */
	{
		alignas(MALLOCATOR_ALIGN) unsigned char geo[sizeof(MNODE_) * 3] = {};
		mnode_* h = (mnode_*)geo;
		const size_ x86_cookie = sizeof(std::size_t);
		h->state = MALLOCATOR::USED | MALLOCATOR::CLASS_ARRAY;  /* new[]: pad_ と cookie 先頭は 0 */
		const unsigned char* elem = geo + sizeof(MNODE_) + x86_cookie;
		assert(((std::uintptr_t)(elem - sizeof(MNODE_)) % MALLOCATOR_ALIGN) != 0);  /* 候補はずれている */
		assert(mnode_locate(elem, x86_cookie) == h);
		h->state = MALLOCATOR::USED;                            /* スカラ new: 直前が真のヘッダ */
		assert(mnode_locate(geo + sizeof(MNODE_), x86_cookie) == h);
	}

	/* --- 6. 静的初期化中の new と全リセット --- */
	assert(mallocator.is_memory(g_early));
	delete[] g_early;
	mallocator.dealloc(p[7]);
	mallocator.dealloc(q2);
	mallocator.dealloc(q3);
	/* ここで share==0 のはず -> current がアリーナ先頭へ戻る */
	unsigned char* z = mallocator.alloc(16);
	assert(z == mallocator_arena + sizeof(MNODE_));  /* share 会計が正確な証明 */
	mallocator.dealloc(z);

	printf("mem_test: all assertions passed\n");
	return 0;
}

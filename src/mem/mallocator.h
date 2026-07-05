#ifndef CPP_DEVEL_MEM_MALLOCATOR_H
#define CPP_DEVEL_MEM_MALLOCATOR_H
/*********************************************
 * mem: region allocator（bump + 再利用 + 前方併合）
 *
 * 原典: devel/include/mallocator.h（実装込み 300行）。
 * devarm の core/mem/mallocator.h（host 移植版）と三方比較して再構成。
 *
 * バッキング: 原典の LINUX 分岐は sbrk、非 LINUX 分岐はリンカラベル
 * (&allocbegin/&allocend) の静的領域だった。host（macOS）では sbrk は
 * 非推奨のため、静的アリーナ（mallocator.cpp の mallocator_arena）を
 * 非 LINUX 分岐の意味論そのままに使う。enlarge() は current_bound を
 * アリーナ内で段階的に進める（原典どおり成長量は size + PAGE_SZ の逓増）。
 *
 * devarm から採用した修正:
 *   - 再利用成立時に frontier(current) と予約(current_bound) を動かさない
 *     （原典は再利用時も enlarge を走らせ予約を無駄に広げていた。bug_log id=95）
 *   - ポインタ演算の uintptr_t 化（原典は unsigned long キャスト）
 *
 * devarm に存在する退行（本実装では原典準拠に戻した。cpp_triage へ報告予定）:
 *   - devarm は非分割再利用でも hole->size = _size と上書きする。端数バイトが
 *     どのノードにも属さなくなり、next_node 走査が破綻し得る。原典どおり
 *     「分割時のみ size を書き換える」。
 *   - devarm は dealloc の冪等ガード（USED でなければ何もしない）を削除した。
 *     double-free で share が壊れる。原典どおりガードする。
 *   - devarm は alloc 時 state=USED+1（refcnt=1）とするが、原典は refcnt=0。
 *     uni_ の refcnt プロトコル（管理対象になる時に incr する）は 0 開始が
 *     前提のため原典に従う。
 *
 * 原典からの現代化:
 *   - 整列: calc_align（sizeof(long)=8 丸め）-> MALLOCATOR_ALIGN(16) 丸め。
 *     C++17 の default new alignment を満たす。
 *   - 例外: throw(std::bad_alloc) 仕様除去（C++17）。エラー種別コードは
 *     std::bad_alloc 派生の alloc_error に保存（原典 BAD_ALLOC(ERR_C(x)) の
 *     コード引数を維持）。
 *   - 静的初期化順序: グローバル mallocator が他 TU の動的初期化から
 *     new 経由で触られても安全なよう、current==nullptr を未初期化の番兵と
 *     して遅延 init する（原典はベアメタルで初期化順を自前管理していた）。
 *   - スレッド安全性は原典どおり無し（単一スレッド前提。ホストツール用途）。
 *
 * 未移植（使用箇所が現れた工程で復活）: is_used / show_remain /
 * stack_to_memory（alen/ancpy 依存、工程2）/ malloc/free/realloc の C 面。
 *********************************************/
#include <new>          /* std::bad_alloc */
#include <cstdint>      /* uintptr_t */
#include <cstdio>       /* printf (info) */
#include "../types.h"
#include "../result.h"
#include "mnode.h"

#define PAGE_SZ 4096u
#ifndef MALLOCATOR_ALLOC_LEAST
#define MALLOCATOR_ALLOC_LEAST 1
#endif
#define TMP_MNODE_LEAST_SZ 16

/* アリーナ容量（テスト・ホストツールは既定 1MiB。必要なら -D で上書き） */
#ifndef MALLOCATOR_ARENA_SZ
#define MALLOCATOR_ARENA_SZ (1024u * 1024u)
#endif
extern unsigned char mallocator_arena[MALLOCATOR_ARENA_SZ];

/* 原典のエラー種別コード付き bad_alloc（BAD_ALLOC(ERR_C(x)) 相当） */
class alloc_error : public std::bad_alloc {
	result_ code_;
 public:
	explicit alloc_error(result_ _code) : code_(_code) {}
	result_ code() const noexcept { return code_; }
	const char* what() const noexcept override { return "mallocator_ allocation failure"; }
};
#define BAD_ALLOC(x) alloc_error(x)

inline size_ malloc_align_up(size_ v){
	return (v + (MALLOCATOR_ALIGN - 1u)) & ~(size_)(MALLOCATOR_ALIGN - 1u);
}

class mallocator_ {
 private:
	unsigned char* current;        /* bump 先端。nullptr = 未初期化の番兵 */
	unsigned char* current_bound;  /* 予約済み上限 */
	MNODE_* reusep;                /* 再利用探索の開始ノード */
	int state;
	size_ size;                    /* enlarge で確保した総容量 */
	size_ share;                   /* 使用中バイト数（ヘッダ込み） */

	MNODE_* next_node(MNODE_* _ref){
		return (MNODE_*)((uintptr_t)_ref + _ref->size + sizeof(MNODE_));
	}
	void ensure_init(){ if (current == nullptr) init(); }

 public:
	mallocator_() : current(nullptr), current_bound(nullptr), reusep(nullptr),
	                state(MALLOCATOR::CLEAN), size(0), share(0) {}
	~mallocator_() {}

	void init(){
		state = MALLOCATOR::CLEAN; size = 0; share = 0;
		current = current_bound = mallocator_arena;
		reusep = (MNODE_*)current_bound;
	}

	void info(){
		printf("begin:%p , current:%p , current_bound:%p , reusep:%p , size:%ld , share:%ld , state:%x\n",
		       (void*)mallocator_arena, (void*)current, (void*)current_bound,
		       (void*)reusep, (long)size, (long)share, (unsigned)state);
	}

	int has_space(){
		size_ diff = (size_)(current - mallocator_arena);
		if ( share < (diff / 2) && diff > (size / 2) ){ state |= MALLOCATOR::HAS_SPACE; }
		else if ( share >= diff ){
			state &= ~MALLOCATOR::HAS_SPACE;
			reusep = (MNODE_*)current_bound;
		}
		return (state & MALLOCATOR::HAS_SPACE);
	}

	/* _size は整列済みであること。分割時のみノードの size を縮める（原典準拠）。 */
	MNODE_* next_reuse_node(size_ _size){
		MNODE_* tmp = reusep;
		while ( (unsigned char*)tmp < current ){
			if ( (tmp->state & MALLOCATOR::USED) == 0 && tmp->size >= _size ){
				if ( tmp->size > _size + sizeof(MNODE_) + TMP_MNODE_LEAST_SZ ){
					reusep = (MNODE_*)((uintptr_t)tmp + _size + sizeof(MNODE_));
					reusep->state &= ~MALLOCATOR::USED;
					reusep->size = tmp->size - _size - sizeof(MNODE_);
					tmp->size = _size;
				}
				return tmp;
			}
			tmp = next_node(tmp);
		}
		return (MNODE_*)nullptr;
	}

	size_ enlarge(){
		if ( current_bound == mallocator_arena + MALLOCATOR_ARENA_SZ )
			throw BAD_ALLOC(ERR_C(MALLOCATOR_LIMIT));
		size_ alloc_size = size + MALLOCATOR_ALLOC_LEAST * PAGE_SZ;   /* 逓増（原典準拠） */
		uintptr_t _addr = (uintptr_t)current_bound + alloc_size;
		if ( _addr < (uintptr_t)mallocator_arena )
			throw BAD_ALLOC(ERR_C(MALLOCATOR_UNDER_BOUND));           /* 桁あふれ */
		uintptr_t _end = (uintptr_t)(mallocator_arena + MALLOCATOR_ARENA_SZ);
		if ( _end < _addr ){
			size += (size_)(_end - (uintptr_t)current_bound);
			current_bound = (unsigned char*)_end;
		} else {
			current_bound = (unsigned char*)_addr;
			size += alloc_size;
		}
		return alloc_size;
	}

	unsigned char* alloc(size_ _size){
		ensure_init();
		if ( _size == 0 ) throw BAD_ALLOC(ERR_C(MALLOCATOR_BAD_ARG_ZERO));
		size_ aligned_size = malloc_align_up(_size);
		if ( aligned_size < _size ) throw BAD_ALLOC(ERR_C(MALLOCATOR_OVER_BOUND)); /* 丸めの桁あふれ */
		size_ require = aligned_size + sizeof(MNODE_);

		/* 再利用パス: 予約済み領域内の空きを使う。frontier/予約は動かさない。
		   share は実ノード占有量（分割後の tmp->size + ヘッダ）で加算する（原典準拠。
		   非分割時は要求より大きい端数込みの占有になる）。 */
		if ( state & MALLOCATOR::HAS_SPACE ){
			MNODE_* tmp = next_reuse_node(aligned_size);
			if ( tmp ){
				tmp->state = MALLOCATOR::USED;   /* refcnt=0（原典準拠） */
				tmp->provided_size = _size;
				tmp->clear_pad();                /* mnode_get 決定的判別の前提 */
				share += tmp->size + sizeof(MNODE_);
				return (unsigned char*)((uintptr_t)tmp + sizeof(MNODE_));
			}
		}

		/* bump パス */
		uintptr_t calc = (uintptr_t)current + require;
		if ( calc > (uintptr_t)(mallocator_arena + MALLOCATOR_ARENA_SZ) )
			throw BAD_ALLOC(ERR_C(MALLOCATOR_OVER_BOUND));
		while ( calc > (uintptr_t)current_bound ){ enlarge(); }
		MNODE_* tmp = (MNODE_*)current;
		tmp->size = aligned_size;
		tmp->state = MALLOCATOR::USED;           /* refcnt=0（原典準拠） */
		tmp->provided_size = _size;
		tmp->clear_pad();                        /* mnode_get 決定的判別の前提 */
		current = (unsigned char*)calc;
		share += require;
		return (unsigned char*)((uintptr_t)tmp + sizeof(MNODE_));
	}

	/* _ref は確保時に返した user ポインタ（＝生ブロック）。operator delete /
	   delete[] はどちらもブロック指針を渡してくるため、ヘッダは常に直前
	   （原典はここでも探りプローブをしていたが、ブロック指針に対しては
	   直前ヘッダが常に正しく、プローブは前ノードのデータを読む危険しかない）。 */
	void dealloc(unsigned char* _ref){
		if ( current == nullptr ) return;
		/* 最小の正当な user ポインタは arena + sizeof(MNODE_)。それ未満は
		   ヘッダ計算が領域外に出る（原典の `< begin` は == begin を素通し
		   していた——境界を厳密化） */
		if ( _ref < mallocator_arena + sizeof(MNODE_) || current_bound <= _ref ) return;
		MNODE_* tmp = mnode_header(_ref);
		if ( !(tmp->state & MALLOCATOR::USED) ) return;   /* 冪等（原典準拠） */
		tmp->state = 0;
		share -= tmp->size + sizeof(MNODE_);
		/* 後続の空きノードを併合（原典準拠: 前方向のみ） */
		MNODE_* tmp2 = next_node(tmp);
		while ( (unsigned char*)tmp2 < current && !(tmp2->state & MALLOCATOR::USED) ){
			tmp->size += tmp2->size + sizeof(MNODE_);
			tmp2 = next_node(tmp);
		}
		if ( share == 0 ){
			current = mallocator_arena;
			reusep = (MNODE_*)current_bound;
		} else if ( tmp < reusep ){
			reusep = tmp;
		}
		has_space();
	}

	result_ is_memory(const void* _ref){
		if ( current == nullptr ) return 0;
		return ( mallocator_arena <= (const unsigned char*)_ref &&
		         (const unsigned char*)_ref < current_bound );
	}
};

extern mallocator_ mallocator;   /* 単一ヒープのシングルトン（定義は mallocator.cpp） */

#endif /* CPP_DEVEL_MEM_MALLOCATOR_H */

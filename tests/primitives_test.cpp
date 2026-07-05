/*********************************************
 * 工程2 検証: 統一プリミティブ層（a 層 / 例外 / check_qty / reference_init /
 * operator_base）の decouple 移植
 *
 * 観測対象:
 *   1. astr: alen/acpy/ancpy/acmp の原典意味論（NUL 終端・ゼロ停止・差分比較）
 *   2. exception: code/line/name の保存（原典で捨てられていた機能の復元）と
 *      std::exception としての捕捉
 *   3. check_qty: char 配列の NUL スロット許容 / 非 char 配列の厳密化（修正点）
 *   4. reference_init: 所有権プロトコル
 *      init_qty=1 開始 -> 共有 init で +1 -> uninit で -1 -> 0 到達で解放
 *      文字列のリテラル深コピー / アリーナ内共有 / qty 指定の切替
 *   5. operator_base: 数量比較の意味論・char 内容比較・要素演算・
 *      qty 不一致 bad_range・文字列連結（null self 修正込み）
 *
 * mem_global_new をリンクし、new/delete が全てアリーナ経由の実配線で検証する。
 * assert 式に副作用を入れない（NDEBUG で消えるため）。
 *********************************************/
#include <cassert>
#include <cstdio>
#include "a/astr.h"
#include "exception.h"
#include "uni/check_qty.h"
#include "uni/reference_init.h"
#include "uni/operator_base.h"
#include "mem/mnode.h"
#include "mem/mallocator.h"

int main(){
	/* --- 1. astr --- */
	assert(alen("") == 0);
	assert(alen("hello") == 5);
	{
		char buf[16];
		qty_ n = acpy("abc", buf);
		assert(n == 3 && buf[0] == 'a' && buf[3] == 0);
		ancpy("abcdef", buf, (size_)4);      /* 4 文字 + NUL */
		assert(buf[3] == 'd' && buf[4] == 0);
		ancpy("ab", buf, (size_)4);          /* src の NUL で停止し NUL 終端 */
		assert(buf[1] == 'b' && buf[2] == 0);
	}
	assert(acmp("abc", "abc") == 0);
	assert(acmp("abd", "abc") > 0);
	assert(acmp("ab",  "abc") < 0);

	/* --- 2. exception --- */
	{
		bool caught = false;
		try { throw bad_range(0x123, 45, "spot"); }
		catch (const devel_exception& e){
			caught = true;
			assert(e.code() == 0x123 && e.line() == 45);
			assert(acmp(e.name(), "spot") == 0);
			assert(acmp(e.what(), "bad_range") == 0);
		}
		assert(caught);
		caught = false;
		try { throw bad_ctor(); }
		catch (const std::exception& e){ caught = true; assert(acmp(e.what(), "bad_ctor") == 0); }
		assert(caught);
	}

	/* --- 4. reference_init: 数値配列の所有権プロトコル --- */
	long* p = nullptr;
	{
		qty_ q = init_qty<long>(&p, 8);
		assert(q == 8);
	}
	assert(p != nullptr && mallocator.is_memory(p));
	assert(mnode_refcnt(p) == 1);                 /* 生成直後 = 1 */
	for ( long i = 0; i < 8; ++i ) assert(p[i] == 0);   /* ゼロ充填 */
	assert(is_qty<long>((const long*)p) == 8);
	assert(is_qty<long>((long)42) == 1);

	long* q = nullptr;
	init<long>(&q, (const long*)p);               /* 共有 */
	assert(q == p);
	assert(mnode_refcnt(p) == 2);

	uninit<long>(q);                              /* 2 -> 1: 生存 */
	assert(mnode_refcnt(p) == 1);
	assert(mnode_header(p)->state & MALLOCATOR::USED);
	uninit<long>(p);                              /* 1 -> 0: 解放 */
	assert(!(mnode_header(p)->state & MALLOCATOR::USED));

	/* 既定容量とエラー */
	{
		long* d = nullptr;
		qty_ qd = init_qty<long>(&d, 0);
		assert(qd == 0x20);
		assert(mnode_qty(d) == 0x20);
		uninit<long>(d);
	}
	{
		bool caught = false;
		long* e = nullptr;
		try { init_qty<long>(&e, -3); } catch (const bad_ctor&){ caught = true; }
		assert(caught);
	}
	{
		long v = 0;
		init_qty<long>(&v, 42);                   /* スカラ: 値の代入 */
		assert(v == 42);
	}

	/* --- 4. 文字列: リテラル深コピー / 共有 / qty 指定 --- */
	char* s = nullptr;
	init<char>(&s, "hello");                      /* リテラル -> 深コピー */
	assert(s != nullptr && mallocator.is_memory(s));
	assert(alen(s) == 5 && s[5] == 0);
	assert(mnode_refcnt(s) == 1);
	assert(mnode_size(s) == 6);                   /* qty+1 の NUL スロット */
	assert(is_qty<char>((const char*)s) == 5);    /* char の qty は alen */

	char* s2 = nullptr;
	init<char>(&s2, (const char*)s);              /* アリーナ内 -> 共有 */
	assert(s2 == s && mnode_refcnt(s) == 2);

	char* s3 = nullptr;
	init<char>(&s3, "world!!", (qty_)3);          /* リテラル + qty 切詰め */
	assert(alen(s3) == 3 && s3[0] == 'w' && s3[2] == 'r' && s3[3] == 0);

	copy<char>(&s2, (const char*)s3);             /* 代入: 旧共有を解放し s3 を共有 */
	assert(mnode_refcnt(s) == 1);
	assert(s2 == s3 && mnode_refcnt(s3) == 2);

	/* --- 4. clone: 深いコピーの独立性 --- */
	{
		long* src = nullptr;
		init_qty<long>(&src, 4);
		for ( long i = 0; i < 4; ++i ) src[i] = i + 10;
		long* dup = nullptr;
		long* r = clone<long>(&dup, (const long*)src);
		assert(r == dup && dup != src);
		assert(mnode_refcnt(dup) == 1 && mnode_refcnt(src) == 1);
		dup[0] = 99;
		assert(src[0] == 10);                     /* 独立 */
		uninit<long>(dup);
		uninit<long>(src);
	}

	/* --- 3. check_qty（char 許容 / 非 char 厳密化） --- */
	assert( check_qty(5L, 5L, s));                /* char*: NUL スロット [qty] 許容 */
	assert(!check_qty(6L, 5L, s));
	{
		long* arr = nullptr;
		init_qty<long>(&arr, 8);
		assert( check_qty(7L, 8L, arr));
		assert(!check_qty(8L, 8L, arr));          /* 修正点: 非 char は [qty] を拒否 */
		assert(!check_qty(-1L, 8L, arr));
		uninit<long>(arr);
	}
	assert(check_qty(1L, 1L, (long)7));           /* スカラ: 原典どおり */

	/* --- 5. operator_base --- */
	{
		long* a = nullptr; init_qty<long>(&a, 4);
		long* b = nullptr; init_qty<long>(&b, 4);
		for ( long i = 0; i < 4; ++i ){ a[i] = i + 1; b[i] = 10; }

		/* 数量比較の意味論（内容ではなく qty を比べる） */
		{ long three = 3; assert(comp_gt<long>((const long*)a, three)); }   /* qty 4 > 3 */
		assert(comp_eq<long>((const long*)a, (const long*)b));   /* qty 4 == qty 4 */

		/* 要素演算 */
		arith_add_assign<long>(a, (const long*)b);
		assert(a[0] == 11 && a[3] == 14);
		arith_sub_assign<long>(a, 1L);
		assert(a[0] == 10 && a[3] == 13);

		/* qty 不一致 -> bad_range */
		{
			long* c = nullptr; init_qty<long>(&c, 2);
			bool caught = false;
			try { arith_add_assign<long>(a, (const long*)c); }
			catch (const bad_range&){ caught = true; }
			assert(caught);
			uninit<long>(c);
		}
		uninit<long>(a);
		uninit<long>(b);
	}

	/* 文字列: 内容比較と連結 */
	{
		char* x = nullptr; init<char>(&x, "abc");
		char* y = nullptr; init<char>(&y, "abc");
		assert(x != y);                            /* 別リテラル由来の別ノード */
		assert(comp_eq<char>((const char*)x, (const char*)y));   /* 内容一致 */

		char* r = arith_add<char>(x, (const char*)y);   /* 連結。x は解放される */
		assert(alen(r) == 6 && acmp(r, "abcabc") == 0);
		assert(mnode_refcnt(r) == 0);              /* 生 new[]: 管理は呼び出し側 */
		{ bool ri = mnode_refcnt_incr(r); assert(ri); }
		uninit<char>(r);
		uninit<char>(y);

		/* null self 連結（原典の null 参照を修正した経路） */
		char* z = arith_add<char>((char*)nullptr, "xyz");
		assert(alen(z) == 3 && acmp(z, "xyz") == 0);
		{ bool zi = mnode_refcnt_incr(z); assert(zi); }
		uninit<char>(z);
	}

	/* 掃除（残りの共有を解放） */
	uninit<char>(s);                              /* s: 1 -> 0 解放 */
	uninit<char>(s2);                             /* s3 共有: 2 -> 1 */
	uninit<char>(s3);                             /* 1 -> 0 解放 */

	printf("primitives_test: all assertions passed\n");
	return 0;
}

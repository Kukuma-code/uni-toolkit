/*********************************************
 * 工程3 検証: uni__<A,B> 本体 + string_ = uni__<char,char*> スモーク
 *
 * 観測対象:
 *   1. string_ の生成（リテラル深コピー）・共有（copy ctor は参照共有）・
 *      代入（旧参照解放 + 共有/深コピー）・自己代入（UAF 修正の検証）
 *   2. 境界: char は NUL スロット [qty] 許容 / 非 char 配列は [qty] で
 *      bad_range(ERR_UNI_OP_PAREN_RANGE)
 *   3. clone/clonep の深いコピー独立性と refcnt の釣り合い
 *      （値渡し戻りがコピー省略の有無に依らず正しいこと）
 *   4. スカラ実体化（long_/char_）の値意味論
 *   5. 配列実体化 uni__<long,long*> の要素演算・qty 不一致 bad_range
 *   6. デストラクタ連鎖で refcnt が 0 に到達し実際に解放されること
 *
 * mem_global_new をリンクし全確保がアリーナ経由の実配線で検証する。
 * assert 式に副作用を入れない。
 *********************************************/
#include <cassert>
#include <cstdio>
#include "uni/uni.h"
#include "uni/string.h"
#include "mem/mnode.h"
#include "mem/mallocator.h"

int main(){
	char* raw_s = nullptr;

	{
		/* --- 1. 生成: リテラルは深コピー --- */
		string_ s("hello");
		raw_s = s();
		assert(raw_s != nullptr && mallocator.is_memory(raw_s));
		assert(s.is_qty() == 5);
		assert(mnode_refcnt(raw_s) == 1);
		assert(s.is_size() == 6);                /* qty+1 の NUL スロット */
		assert(s[0] == 'h' && s[4] == 'o');
		assert(s[5] == 0);                       /* NUL スロット読みは許容 */

		/* 境界: [6] は範囲外 */
		{
			bool caught = false;
			try { (void)s[6]; }
			catch (const bad_range& e){ caught = true; assert(e.code() == ERR_UNI_OP_PAREN_RANGE); }
			assert(caught);
		}

		/* --- 1. copy ctor は共有（refcnt +1）、書き込みは両者に見える --- */
		string_ t(s);
		assert(t() == raw_s);
		assert(mnode_refcnt(raw_s) == 2);
		t[0] = 'H';
		assert(s[0] == 'H');

		/* --- 1. 代入: 共有追加 -> リテラル代入で切替 --- */
		string_ u;                               /* char の既定は null node */
		assert(u() == nullptr);
		u = t;
		assert(u() == raw_s && mnode_refcnt(raw_s) == 3);
		{ string_& uref = u; u = uref; }         /* 自己代入: UAF 修正の検証 */
		assert(u() == raw_s && mnode_refcnt(raw_s) == 3);
		u = "fresh";                             /* 旧参照を解放して深コピー */
		assert(mnode_refcnt(raw_s) == 2);
		assert(u() != raw_s && u.is_qty() == 5 && mnode_refcnt(u()) == 1);

		/* --- 3. clone: 深いコピーの独立性 --- */
		string_ c(s.clone());
		assert(c() != raw_s);
		assert(mnode_refcnt(c()) == 1);          /* コピー省略の有無に依らず 1 */
		assert(mnode_refcnt(raw_s) == 2);        /* 元は不変 */
		c[0] = 'X';
		assert(s[0] == 'H');

		/* clonep: ヒープ上の複製（wrapper は生 new） */
		string_* cp = s.clonep();
		assert((*cp)() != raw_s);
		assert(mnode_refcnt((*cp)()) == 1);
		delete cp;                               /* dtor -> uninit -> 解放 */

		/* 部分列 ctor（アリーナ内・qty 不一致 -> 新規確保して複写） */
		string_ sub(s, (qty_)3);
		assert(sub() != raw_s && sub.is_qty() == 3);
		assert(sub[0] == 'H' && sub[2] == 'l' && sub[3] == 0);

		/* p() の煙試験（出力の体裁は目視。クラッシュしないこと）。
		   nullptr は工程4 の出力層（aprn.h の p 群）と曖昧になるため明示キャスト */
		printf("p(&s) -> ");
		p(&s, -1, FORMAT::NEWLINE);
		p((string_*)nullptr);
		putchar('\n');
	}
	/* --- 6. スコープ終了: 共有が解けて解放される --- */
	assert(!(mnode_header(raw_s)->state & MALLOCATOR::USED));

	/* --- 4. スカラ実体化 --- */
	{
		long_ x(5);
		x += 3;
		assert(x() == 8);
		assert(x > 2L);
		assert(x == 8L);
		long_ y(x);                              /* スカラは値コピー */
		y += 1;
		assert(y() == 9 && x() == 8);
		long_ z(x + y);                          /* 8 + 9 */
		assert(z() == 17);

		char_ ch((qty_)'a');
		assert(ch() == 'a');
		ch += (char)1;
		assert(ch() == 'b');
	}

	/* --- 5. 配列実体化 --- */
	{
		uni__<long, long*> arr((qty_)4);
		assert(arr.is_qty() == 4);
		assert(arr.is_size() == 32);
		for ( idx_ i = 0; i < 4; ++i ) assert(arr[i] == 0);   /* ゼロ充填 */
		arr[2] = 42;
		assert(arr[2] == 42);
		{
			bool caught = false;
			try { (void)arr[4]; }                /* 非 char は [qty] を拒否（修正点） */
			catch (const bad_range&){ caught = true; }
			assert(caught);
		}

		uni__<long, long*> shared(arr);          /* 共有 */
		assert(shared() == arr());
		assert(mnode_refcnt(arr()) == 2);

		uni__<long, long*> deep(arr.clone());    /* 深いコピー */
		assert(deep() != arr());
		deep += arr;                             /* 要素ごと加算（qty 一致） */
		assert(deep[2] == 84 && deep[0] == 0);
		assert(arr[2] == 42);

		{
			bool caught = false;
			uni__<long, long*> small((qty_)2);
			try { deep += small; }               /* qty 不一致 */
			catch (const bad_range&){ caught = true; }
			assert(caught);
		}

		/* 既定 ctor は容量 0x20（原典仕様） */
		uni__<long, long*> dflt;
		assert(dflt.is_qty() == 0x20);
	}

	printf("uni_test: all assertions passed\n");
	return 0;
}

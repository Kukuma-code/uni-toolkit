/*********************************************
 * 工程4 検証: 派生層一式
 *   qty__ 双子 / lhdr__ / lnode__ / allocator_node__ / allocator_element__ /
 *   element__（estring_）/ ballocator__ / list__（slist_ + スカラ list）
 *
 * 観測対象:
 *   1. codegen 双子（qty__ / base_qty__ / num__ / type__）の値意味論と
 *      operator=(const A*) の逆参照修正
 *   2. エラーコードの LP64 符号拡張（負値契約）と is_check_idx の整合
 *   3. lhdr__ の接続プロトコル（connect / connect_f / connect_i）
 *   4. lnode__ = uni_ 値 + 連結の合成（string_ 実体化）
 *   5. allocator_node__ / allocator_element__（uni__/element__ の姉妹）の
 *      台帳と共有 refcnt
 *   6. element__（estring_）: D 台帳（使用数）と C 確保数の分離、
 *      リテラル代入時の台帳更新（is_qty 非テンプレート修正の検証）
 *   7. ballocator__ のスラブ供給・累計・スロット枯渇
 *   8. list__: add / 添字 / 検索（内容比較）/ del / insert / enlarge /
 *      simple_sort（スカラは値順、文字列は uni_ の数量比較）
 *   9. スコープ終了で slab とノードがアリーナへ返ること
 *
 * mem_global_new をリンクし全確保がアリーナ経由の実配線で検証する。
 * assert 式に副作用を入れない。
 *********************************************/
#include <cassert>
#include <cstdio>
#include "result.h"
#include "cond.h"
#include "uni/uni.h"
#include "uni/string.h"
#include "uni/qty.h"
#include "uni/base_qty.h"
#include "uni/num.h"
#include "uni/type.h"
#include "uni/lnode.h"   /* lhdr.h は lnode 経由（接続プロトコルは lnode で検証） */
#include "uni/allocator_node.h"
#include "uni/allocator_element.h"
#include "uni/element.h"
#include "uni/ballocator.h"
#include "uni/list.h"
#include "mem/mnode.h"
#include "mem/mallocator.h"

int main(){
	/* --- 2. エラーコードの負値契約（LP64 符号拡張） --- */
	assert(ERR_N(BALLOC_LIST_OVER) < 0);
	assert(ERR_N(LIST_SEARCH_IDX_FAIL) < 0);
	assert(!is_check_idx(ERR_N(BALLOC_LIST_OVER)));
	assert(is_check_idx(0));

	/* --- 1. qty__ 双子 --- */
	{
		qty__<long> q(5);
		q += 3;
		assert(q.is_qty() == 8);
		assert(q == 8L);
		assert(q > 2L);
		assert(q() == 8);
		assert(q.check_qty(8));                  /* CHECK_LIMIT は閉区間 */
		assert(!q.check_qty(9));
		assert(q.qtying(10) == 8);
		assert(q.qtying(-1) == 0);
		assert(q.qtying(3) == 3);
		{
			long v = 42;
			q = &v;                              /* 逆参照修正の検証 */
			assert(q.is_qty() == 42);
		}
		qty__<long> r(q.clone());
		r -= 2;
		assert(r.is_qty() == 40 && q.is_qty() == 42);
		assert(q != r);
		assert(q > r);
		qty__<long> s(q + r);                    /* 42 + 40 */
		assert(s.is_qty() == 82);
		q.qty_is(4);
		assert(q.is_qty() == 4);
		q.clear();
		assert(q.is_qty() == 0);

		/* 双子の同型性（各双子固有のアクセサ名で同じ振る舞い） */
		base_qty__<long> bq(7);
		bq += 1;
		assert(bq.is_base_qty() == 8 && bq.check_base_qty(8) && !bq.check_base_qty(9));
		assert(bq.base_qtying(100) == 8);
		num__<long> nm(7);
		nm *= 2;
		assert(nm.is_num() == 14 && nm.check_num(14));
		nm.num_is(3);
		assert(nm.is_num() == 3);
		type__<long> ty(2);
		ty -= 2;
		assert(ty.is_type() == 0 && ty.check_type(0));
		ty.type_is(9);
		assert(ty.typeing(4) == 4 && ty.typeing(12) == 9);   /* 原典名 typeing（codegen の機械連結） */
	}

	/* --- 3. lhdr__ 接続プロトコル（スカラ lnode の 3 ノード輪） --- */
	{
		typedef lnode__<long, long, uni__<long, long>> node_t;
		node_t n1((qty_)1), n2((qty_)2), n3((qty_)3);
		n1.connect(&n1, &n2);
		n1.connect(&n2, &n3);
		n1.connect(&n3, &n1);
		assert(n1.is_next() == &n2 && n2.is_next() == &n3 && n3.is_next() == &n1);
		assert(n2.is_prev() == &n1 && n3.is_prev() == &n2);
		n1.connect_f(&n2);                       /* n2 を外す */
		assert(n1.is_next() == &n3 && n3.is_prev() == &n1);
		n1.connect_i(&n3, &n2);                  /* n3 の直前へ戻す */
		assert(n1.is_next() == &n2 && n2.is_next() == &n3 && n3.is_prev() == &n2);

		/* 4. lnode の値部（スカラ）: uni_ 経由の値演算 */
		assert(n1() == 1 && n2() == 2);
		n2 += 10L;
		assert(n2() == 12);
		assert(n2 > n1);
	}

	/* --- 4. lnode__（string_ 実体化）: 値の深コピーと連結の複写規約 --- */
	{
		typedef lnode__<char, char*, string_> snode_t;
		snode_t a("abc");
		assert((a.uni__<char, char*>::is_qty() == 3));
		assert(a[0] == 'a' && a[2] == 'c');
		snode_t b(a);                            /* copy ctor: C は共有、lhdr は複写 */
		assert(b() == a());
		assert(mnode_refcnt(a()) == 2);
		snode_t c((qty_)0);
		c = "xyz";
		assert(c() != nullptr && c[1] == 'y');
		assert(a == a);                          /* 内容比較（comp_eq<char>） */
	}

	/* --- 5. allocator_node__ / allocator_element__ --- */
	{
		allocator_node__<long, long*> an((qty_)4);
		assert(an.is_qty() == 4);
		assert(mallocator.is_memory(an.is_allocator_node()));
		assert(mnode_refcnt(an()) == 1);
		an[2] = 7;
		assert(an[2] == 7);
		{
			bool caught = false;
			try { (void)an[4]; }
			catch (const bad_range& e){ caught = true; assert(e.code() == ERR_UNI_OP_PAREN_RANGE); }
			assert(caught);
		}
		allocator_node__<long, long*> sh(an);    /* 共有 */
		assert(sh() == an() && mnode_refcnt(an()) == 2);
		allocator_node__<long, long*> dp(an.clone());
		assert(dp() != an() && dp[2] == 7);
		dp[2] = 9;
		assert(an[2] == 7);

		allocator_element__<long, long*> ae((qty_)6);
		assert(ae.is_base_qty() == 6);                            /* D 台帳 */
		assert((ae.allocator_node__<long, long*>::is_qty() == 6)); /* C 確保数 */
		ae[5] = 11;
		assert(ae[5] == 11);
		assert(ae.base_qtying(100) == 6);
	}

	/* --- 6. element__（estring_）--- */
	{
		estring_ s("hello");
		assert(s.is_qty() == 5);                                 /* D 台帳 */
		assert((s.uni__<char, char*>::is_qty() == 5));           /* C 側 */
		assert(s[0] == 'h' && s[4] == 'o' && s[5] == 0);
		assert(mnode_refcnt(s()) == 1);

		estring_ t(s);                                           /* C 共有 + D 複写 */
		assert(t() == s() && mnode_refcnt(s()) == 2);
		assert(t.is_qty() == 5);

		s = "reassigned";                                        /* リテラル代入: D 台帳更新の修正検証 */
		assert(s.is_qty() == 10);
		assert(s() != t() && mnode_refcnt(t()) == 1);

		s = (char)'x';                                           /* スカラ代入: 先頭書き換え + D=1 */
		assert(s.is_qty() == 1);
		assert(s[0] == 'x');

		estring_ c(t.clone());                                   /* 深いコピー */
		assert(c() != t() && c.is_qty() == 5);
		c[0] = 'H';
		assert(t[0] == 'h');

		/* 数量配列 element: 複合代入は C と D の両方に作用（原典仕様） */
		element__<long, long*> e1((qty_)4);
		element__<long, long*> e2((qty_)4);
		e1[1] = 5;
		e2[1] = 6;
		e1 += e2;
		assert(e1[1] == 11);
		assert(e1.is_qty() == 8);                                /* D 4+=4 */
		assert((e1.uni__<long, long*>::is_qty() == 4));          /* C は不変 */

		printf("estring info -> ");
		c.info();
	}

	/* --- 7. ballocator__（スロット 4 の小さな束で枯渇まで） --- */
	{
		ballocator__<long> ba((qty_)4);
		assert(ba.is_base_qty() == 4);
		assert(ba.is_qty() == 0);
		long* s1 = ba.balloc(8);
		assert(s1 != nullptr && mallocator.is_memory(s1));
		assert(mnode_refcnt(s1) == 1);
		assert(ba.is_qty() == 8);
		long* s2 = ba.balloc(8);
		long* s3 = ba.balloc(8);
		long* s4 = ba.balloc(8);
		assert(s2 && s3 && s4);
		assert(ba.is_qty() == 32);
		assert(!ba.is_full());                   /* 原典仕様: 空きなし -> false */
		long* s5 = ba.balloc(8);
		assert(s5 == nullptr);                   /* スロット枯渇 */
		assert(ba.is_qty() == 32);
	}

	/* --- 8a. slist_: add / 検索 / del / insert / enlarge --- */
	/* スラブは new lnode__[qty]（非トリビアル型の配列 = cookie 付き）の先頭要素。
	   ヘッダは型付きの mnode_get で引く（untyped の mnode_header は cookie を知らず、
	   cookie ぶん手前のずれた位置を読む） */
	lnode__<char, char*, string_>* slab0 = nullptr;
	char* node0 = nullptr;
	{
		slist_ sl;                               /* 初期スラブ 0x20 ノード */
		assert(sl.is_qty() == 0);
		assert(sl.is_base_qty() == 0x20);

		string_* p_aa = sl.add((char*)"aa");
		assert(p_aa != nullptr);
		string_* p_bb = sl.add((char*)"bbb");
		string_* p_cc = sl.add((char*)"cccc");
		assert(p_bb != nullptr && p_cc != nullptr);
		assert(sl.is_qty() == 3);
		slab0 = sl.head();
		assert(is_mnode_class_array(mnode_get(slab0)));   /* 判別が真のヘッダに届く */
		node0 = (*sl.head())();
		assert(mallocator.is_memory(node0));

		/* 添字と内容（リテラルは深コピーされている） */
		assert(sl[0]() != nullptr && sl[0][0] == 'a');
		assert(sl[1][2] == 'b');
		assert((sl[2].uni__<char, char*>::is_qty() == 4));

		/* 検索は内容比較（comp_eq<char> = acmp）。C の値渡し/参照渡しの
		   オーバーロードは lvalue で曖昧になるためポインタ渡し（原典の
		   呼び出し面と同じ） */
		string_ key("bbb");
		string_* hit = sl.search(&key);
		assert(hit == p_bb);
		assert(sl.search_t_ndx(&key) == 1);
		{
			string_ miss("zzz");
			ndx_ ndx = sl.search_t_ndx(&miss);
			assert(ndx == ERR_N(LIST_SEARCH_IDX_FAIL));
			assert(!is_check_idx(ndx));
			string_* none = sl.search(&miss);
			assert(none == nullptr);
		}

		/* del: ノードは空き鎖へ返り、後続 add が再利用する */
		{
			string_ bbb("bbb");
			result_ r = sl.del(&bbb);
			assert(r == TRUE);
		}
		assert(sl.is_qty() == 2);
		assert(sl[0][0] == 'a' && sl[1][0] == 'c');
		string_* p_dd = sl.add((char*)"dd");
		assert(p_dd != nullptr && sl.is_qty() == 3);
		assert(sl[2][0] == 'd');

		/* insert: [1] の直前へ */
		{
			lnode__<char, char*, string_>* at = &sl[1];
			result_ r = sl.insert(at, (char*)"ins");
			assert(r == TRUE);
		}
		assert(sl.is_qty() == 4);
		assert(sl[0][0] == 'a' && sl[1][0] == 'i' && sl[2][0] == 'c' && sl[3][0] == 'd');

		/* del_byidx */
		{
			result_ r = sl.del_byidx(1);
			assert(r == TRUE);
		}
		assert(sl.is_qty() == 3);
		assert(sl[1][0] == 'c');

		/* enlarge: 初期容量 0x20 を超えて追加 */
		{
			char buf[8];
			for ( long i = sl.is_qty(); i < 0x20 + 5; ++i ){
				buf[0] = (char)('A' + (i % 26)); buf[1] = 0;
				string_* added = sl.add(buf);    /* スタック文字列 -> 深コピー */
				assert(added != nullptr);
			}
		}
		assert(sl.is_qty() == 0x20 + 5);
		assert(sl.is_base_qty() == 0x20 + 0x40); /* enlarge は倍量スラブを継ぎ足す */
		assert(sl[0x20 + 4][0] == (char)('A' + ((0x20 + 4) % 26)));
		assert(sl.head()->is_prev() != nullptr);
		assert(sl.tail() != nullptr);

		printf("slist info -> ");
		sl.info();
	}
	/* --- 9. スコープ終了: 先頭スラブと文字列ノードがアリーナへ返る --- */
	assert(!(mnode_get(slab0)->state & MALLOCATOR::USED));
	assert(!(mnode_header(node0)->state & MALLOCATOR::USED));

	/* --- 8b. スカラ list: add_t_ndx / simple_sort / move --- */
	{
		list__<long> ll((qty_)8);
		ll.add(3L);
		ll.add(1L);
		ll.add(2L);
		assert(ll.is_qty() == 3);
		assert(ll[0]() == 3 && ll[1]() == 1 && ll[2]() == 2);
		{
			ndx_ ndx = ll.add_t_ndx(9L);
			assert(ndx == 3);
		}
		{
			long* found = nullptr;
			uni__<long, long>* u = ll.search(2L);
			assert(u != nullptr);
			(void)found;
		}
		{
			result_ r = ll.del(9L);
			assert(r == TRUE);
			assert(ll.is_qty() == 3);
		}

		result_ sorted = ll.simple_sort();
		assert(sorted == TRUE);
		assert(ll[0]() == 1 && ll[1]() == 2 && ll[2]() == 3);

		/* move_before: 末尾要素を先頭の前へ */
		{
			lnode__<long, long, uni__<long, long>>* mv = &ll[2];
			lnode__<long, long, uni__<long, long>>* to = &ll[0];
			result_ r = ll.move_before(mv, to);
			assert(r == TRUE);
		}
		assert(ll[0]() == 3 && ll[1]() == 1 && ll[2]() == 2);
		{
			lnode__<long, long, uni__<long, long>>* mv = &ll[0];
			lnode__<long, long, uni__<long, long>>* to = &ll[2];
			result_ r = ll.move_after(mv, to);
			assert(r == TRUE);
		}
		assert(ll[0]() == 1 && ll[1]() == 2 && ll[2]() == 3);
	}

	/* --- p()/info() の煙試験（体裁は目視。クラッシュしないこと） --- */
	{
		uni__<long, long*> arr((qty_)4);
		arr[0] = 10; arr[1] = 20; arr[2] = 30; arr[3] = 40;
		printf("arr.p(0,3) -> ");
		arr.p((idx_)0, (idx_)3, FORMAT::SEPARATOR | FORMAT::NEWLINE);
		printf("arr.info -> ");
		arr.info();
		long_ x(5);
		printf("long_.info -> ");
		x.info();
		string_ s("smoke");
		printf("p(uni) -> ");
		p(s, FORMAT::NEWLINE);
	}

	printf("derived_test: all assertions passed\n");
	return 0;
}

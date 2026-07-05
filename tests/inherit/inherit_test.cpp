/*********************************************
 * 目的2 検証: 生成した継承オブジェクトが意図どおり動くか
 *
 * inherit_fixture.h の全メソッド（probe が使わない operator+ / == /
 * doubled / reset / clone も含む）を実際に呼び、値意味論・多重継承の
 * 委譲・using 宣言・CRTP リンクが正しく機能することを assert で確認する。
 *
 * ここでは「仕様書に書いた機能が呼べば正しく動く」ことを見る。
 * 「呼ばなければ実体化されない」ことは symcheck（probe 側）が見る。
 * assert 式に副作用を入れない。
 *********************************************/
#include <cassert>
#include <cstdio>
#include "inherit/inherit_fixture.h"

int main(){
	/* --- 単一基底 holder_ --- */
	{
		holder_<long> h(5);
		assert(h.get() == 5);
		h.set(9);
		assert(h.get() == 9);
		assert(h.doubled() == 18);
		holder_<long> a(2), b(3);
		holder_<long> c = a + b;          /* operator+ */
		assert(c.get() == 5);
		assert(!(a == b) && (a == holder_<long>(2)));
	}

	/* --- 単一基底 counter_ --- */
	{
		counter_<long> k;
		assert(k.count() == 0);
		k.tick(); k.tick();
		assert(k.count() == 2);
		k.reset();
		assert(k.count() == 0);
	}

	/* --- 多重継承の合成 cell_（C=holder_, D=counter_） --- */
	{
		cell_<long> cell(3);
		assert(cell.get() == 3);          /* using C::get */
		assert(cell.count() == 0);        /* using D::count */
		cell.put(10);                     /* C::set + D::tick */
		assert(cell.get() == 10 && cell.count() == 1);
		cell.put(20);
		assert(cell.get() == 20 && cell.count() == 2);
		assert(cell.sum() == 22);         /* C::get + D::count */

		cell_<long> dup = cell.clone();   /* clone: 値のみ複写、カウンタは初期化 */
		assert(dup.get() == 20 && dup.count() == 0);
		dup.put(99);
		assert(cell.get() == 20);         /* 独立 */
	}

	/* --- CRTP 侵入型リンク node_ --- */
	{
		node_<long> a(1), b(2), c(3);
		a.link(&b); b.link(&c);
		assert(a.get() == 1);
		assert(a.next() == &b && a.next()->get() == 2);
		assert(a.next()->next() == &c && a.next()->next()->get() == 3);
		assert(c.next() == nullptr);
	}

	printf("inherit_test: all assertions passed\n");
	return 0;
}

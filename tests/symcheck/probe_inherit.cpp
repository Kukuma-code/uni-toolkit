/*********************************************
 * symcheck 被験体（汎用継承フィクスチャ版）
 *
 * inherit_fixture.h の「使う想定」メソッドだけを odr-use する最小 TU。
 * これを .o にコンパイルしたシンボル表を symcheck が検査する:
 *   目的1: 使ったメソッドが実体化されている（present）
 *   目的3: 使っていないメソッド（operator+ / == / doubled / reset / clone）が
 *          実体化されていない（absent）
 *
 * main は持たない（純粋な実体化トリガ）。最適化で消えないよう戻り値を残す。
 *********************************************/
#include "inherit/inherit_fixture.h"

long probe_cell(){
	cell_<long> c(3);          /* cell_ ctor -> C(x), D(0) */
	c.put(10);                 /* C::set + D::tick */
	return c.sum();            /* C::get + D::count（using 経由） */
}

long probe_node(){
	node_<long> a(1), b(2);
	a.link(&b);                /* linkable_::link */
	return a.get() + a.next()->get();   /* holder_::get, linkable_::next */
}

/* 外から可視にして最適化除去を防ぐ集約点（実行はされない） */
long probe_inherit_anchor(){ return probe_cell() + probe_node(); }

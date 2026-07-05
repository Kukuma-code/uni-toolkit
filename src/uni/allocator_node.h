#ifndef CPP_DEVEL_UNI_ALLOCATOR_NODE_H
#define CPP_DEVEL_UNI_ALLOCATOR_NODE_H
/*********************************************
 * allocator_node__<A,B> — ノードホルダ（原典 backup/tmp_include/allocator_node.h）
 *
 * uni__ と同じ「node メンバ + 自由関数層委譲」構造の codegen 姉妹で、
 * メンバ名が allocator_node、アクセサが is_allocator_node である点だけが
 * 異なる（uni.h の設計注記を参照）。ballocator__ の C 基底（スラブ
 * ポインタ表の台帳）として allocator_element__ 経由で使われる。
 *
 * uni__ との差分（原典どおり保存）:
 *   - ctor 群に「(uni 相当, qty, idx) の 3 引数」系がある
 *   - p()/info()/qtying/check_qty は uni__ と同型
 *   - スカラ typedef 群は持たない
 *
 * 原典からの現代化は uni.h と同一方針（throw 仕様除去・flatten 非採用・
 * D_UNI 計装除去・PRE トークン連結の実名化）。
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "../result.h"
#include "../format.h"
#include "../exception.h"
#include "../mem/mnode.h"
#include "../a/aprn.h"
#include "reference_init.h"
#include "operator_base.h"
#include "check_qty.h"

template <typename A, typename B = A*>
class allocator_node__ {
 protected:
	B allocator_node;
 public:
	/* --- ctor 群（init/init_qty へ委譲） --- */
	allocator_node__(const A* A_allocator_node){ init<A>(&allocator_node, A_allocator_node); }
	allocator_node__(const A* A_allocator_node, const qty_ A_allocator_node_qty){ init<A>(&allocator_node, A_allocator_node, A_allocator_node_qty); }
	allocator_node__(const qty_ A_allocator_node_qty = 0){ init_qty<A>(&allocator_node, A_allocator_node_qty); }

	allocator_node__(const allocator_node__<A, A>& _allocator_node){ init<A>(&allocator_node, _allocator_node.is_allocator_node()); }
	allocator_node__(const allocator_node__<A, A>& _allocator_node, qty_ _allocator_node_qty){ init<A>(&allocator_node, _allocator_node.is_allocator_node(), _allocator_node_qty, 0); }
	allocator_node__(const allocator_node__<A, A>& _allocator_node, qty_ _allocator_node_qty, idx_ _allocator_node_idx){ init<A>(&allocator_node, _allocator_node.is_allocator_node(), _allocator_node_qty, _allocator_node_idx); }
	allocator_node__(const allocator_node__<A, A>* _allocator_node){ init<A>(&allocator_node, _allocator_node->is_allocator_node()); }
	allocator_node__(const allocator_node__<A, A>* _allocator_node, qty_ _allocator_node_qty){ init<A>(&allocator_node, _allocator_node->is_allocator_node(), _allocator_node_qty, 0); }
	allocator_node__(const allocator_node__<A, A>* _allocator_node, qty_ _allocator_node_qty, idx_ _allocator_node_idx){ init<A>(&allocator_node, _allocator_node->is_allocator_node(), _allocator_node_qty, _allocator_node_idx); }

	allocator_node__(const allocator_node__<A, A*>& _allocator_node){ init<A>(&allocator_node, _allocator_node.is_allocator_node()); }
	allocator_node__(const allocator_node__<A, A*>& _allocator_node, qty_ _allocator_node_qty){ init<A>(&allocator_node, _allocator_node.is_allocator_node(), _allocator_node_qty); }
	allocator_node__(const allocator_node__<A, A*>* _allocator_node){ init<A>(&allocator_node, _allocator_node->is_allocator_node()); }
	allocator_node__(const allocator_node__<A, A*>* _allocator_node, qty_ _allocator_node_qty){ init<A>(&allocator_node, _allocator_node->is_allocator_node(), _allocator_node_qty); }

	~allocator_node__(){ ::uninit<A>(allocator_node); }

	/* --- 比較（vs 生値） --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(allocator_node, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(allocator_node, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(allocator_node, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(allocator_node, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(allocator_node, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(allocator_node, _ref); }

	/* --- 算術（vs 生値） --- */
	allocator_node__<A, B> operator+(const A& _ref){ allocator_node__<A, B> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	allocator_node__<A, B> operator-(const A& _ref){ allocator_node__<A, B> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	allocator_node__<A, B> operator/(const A& _ref){ allocator_node__<A, B> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	allocator_node__<A, B> operator*(const A& _ref){ allocator_node__<A, B> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	allocator_node__<A, B>& operator=(const A& _ref){ copy<A>(&allocator_node, _ref); return *this; }
	allocator_node__<A, B>& operator=(const A* _ref){ copy<A>(&allocator_node, _ref); return *this; }
	allocator_node__<A, B>& operator+=(const A& _ref){ arith_add_assign<A>(allocator_node, _ref); return *this; }
	allocator_node__<A, B>& operator-=(const A& _ref){ arith_sub_assign<A>(allocator_node, _ref); return *this; }
	allocator_node__<A, B>& operator/=(const A& _ref){ arith_div_assign<A>(allocator_node, _ref); return *this; }
	allocator_node__<A, B>& operator*=(const A& _ref){ arith_mul_assign<A>(allocator_node, _ref); return *this; }

	/* --- 比較・算術（vs allocator_node__） --- */
	result_ operator> (const allocator_node__<A, B>& _ref){ if ( allocator_node != _ref.allocator_node ) return comp_gt<A>(allocator_node, _ref.allocator_node); return 0; }
	result_ operator< (const allocator_node__<A, B>& _ref){ if ( allocator_node != _ref.allocator_node ) return comp_lt<A>(allocator_node, _ref.allocator_node); return 0; }
	result_ operator>=(const allocator_node__<A, B>& _ref){ return comp_ge<A>(allocator_node, _ref.allocator_node); }
	result_ operator<=(const allocator_node__<A, B>& _ref){ return comp_le<A>(allocator_node, _ref.allocator_node); }
	result_ operator==(const allocator_node__<A, B>& _ref){ return comp_eq<A>(allocator_node, _ref.allocator_node); }
	result_ operator!=(const allocator_node__<A, B>& _ref){ return comp_ne<A>(allocator_node, _ref.allocator_node); }

	allocator_node__<A, B> operator+(const allocator_node__<A, B>& _ref){ allocator_node__<A, B> tmp(this->clone()); arith_add_assign<A>(tmp.allocator_node, _ref.allocator_node); return tmp; }
	allocator_node__<A, B> operator-(const allocator_node__<A, B>& _ref){ allocator_node__<A, B> tmp(this->clone()); arith_sub_assign<A>(tmp.allocator_node, _ref.allocator_node); return tmp; }
	allocator_node__<A, B> operator/(const allocator_node__<A, B>& _ref){ allocator_node__<A, B> tmp(this->clone()); arith_div_assign<A>(tmp.allocator_node, _ref.allocator_node); return tmp; }
	allocator_node__<A, B> operator*(const allocator_node__<A, B>& _ref){ allocator_node__<A, B> tmp(this->clone()); arith_mul_assign<A>(tmp.allocator_node, _ref.allocator_node); return tmp; }

	allocator_node__<A, B>& operator=(const allocator_node__<A, B>& _ref){ copy<A>(&allocator_node, _ref.allocator_node); return *this; }
	allocator_node__<A, B>& operator=(const allocator_node__<A, B>* _ref){ copy<A>(&allocator_node, _ref->allocator_node); return *this; }
	allocator_node__<A, B>& operator+=(const allocator_node__<A, B>& _ref){ arith_add_assign<A>(allocator_node, _ref.allocator_node); return *this; }
	allocator_node__<A, B>& operator-=(const allocator_node__<A, B>& _ref){ arith_sub_assign<A>(allocator_node, _ref.allocator_node); return *this; }
	allocator_node__<A, B>& operator/=(const allocator_node__<A, B>& _ref){ arith_div_assign<A>(allocator_node, _ref.allocator_node); return *this; }
	allocator_node__<A, B>& operator*=(const allocator_node__<A, B>& _ref){ arith_mul_assign<A>(allocator_node, _ref.allocator_node); return *this; }

	/* --- アクセス --- */
	B& operator()() const { return (B&)this->allocator_node; }
	A& operator()(const idx_ _idx){ return operator()(this->allocator_node, _idx); }
	A& operator[](const idx_ _idx){ return operator()(this->allocator_node, _idx); }
	A& operator()(A& _ref, const idx_ _idx){ (void)_ref; (void)_idx; return (A&)this->allocator_node; }
	A& operator()(A* _ref, const idx_ _idx){
		(void)_ref;
		if ( !this->check_qty(_idx) ) throw bad_range(ERR_N(UNI_OP_PAREN_RANGE));
		return this->allocator_node[_idx];
	}

	/* --- 複製 --- */
	allocator_node__<A, B> clone(){
		allocator_node__<A, B> tmp;
		::clone<A>(&tmp.allocator_node, this->allocator_node);
		return tmp;
	}
	allocator_node__<A, B>* clonep(){
		allocator_node__<A, B>* tmp = new allocator_node__<A, B>;
		::clone<A>(&tmp->allocator_node, this->allocator_node);
		return tmp;
	}

	/* --- 状態 --- */
	bool check_qty(idx_ _ref){ return ::check_qty(_ref, this->is_qty(), this->allocator_node); }
	void clear(){ this->clear(this->allocator_node); }
	void clear(A&){ this->allocator_node = 0; }
	void clear(A*){ mnode_refcnt_decr_or_delete(this->allocator_node); this->allocator_node = 0; }
	B& is_allocator_node() const { return (B&)this->allocator_node; }
	qty_ is_qty() const { return ::is_qty<A>(allocator_node); }
	size_ is_size() const { return mnode_size((unsigned char*)this->allocator_node); }
	qty_ qtying(qty_ _qty){
		qty_ tmpqty = this->is_qty();
		if ( _qty < 0 ) _qty = 0;
		else if ( _qty > tmpqty ) _qty = tmpqty;
		return _qty;
	}

	/* --- 出力 --- */
	void p(idx_ _idx = 0){
		if ( this->check_qty(_idx) ) ::p(allocator_node, _idx);
		else putchar('*');
	}
	void p(idx_ _begin, idx_ _end, status_ _status = FORMAT::SEPARATOR){
		if ( !allocator_node ) return;
		::p(allocator_node, this->is_qty(), _begin, _end, _status);
	}
	void info(A* _ref, status_ _status){
		(void)_ref;
		qty_ _qty = ::is_qty<A>(this->allocator_node);
		printf("<allocator_node__><qty:%05ld> value:", _qty);
		if ( _qty >= 7 ){ this->p((idx_)0, (idx_)6); if ( _qty >= 8 ) puts("..."); }
		else { this->p((idx_)0, (idx_)_qty); }
		newline(_status);
	}
	void info(A _ref, status_ _status){
		(void)_ref;
		printf("<allocator_node__><qty:%08ld> ", this->is_qty());
		printf("value:");
		::p(allocator_node);
		newline(_status);
	}
	void info(status_ _status = FORMAT::NEWLINE){ this->info(allocator_node, _status); }
};

/* クラス全体の出力（原典 allocator_node.h:192） */
template <typename A, typename B>
inline void p(const allocator_node__<A, B>& _ref, status_ _status = 0){
	((allocator_node__<A, B>&)_ref).p();
	newline(_status);
}

#endif /* CPP_DEVEL_UNI_ALLOCATOR_NODE_H */

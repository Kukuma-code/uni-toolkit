#ifndef CPP_DEVEL_UNI_QTY_H
#define CPP_DEVEL_UNI_QTY_H
/*********************************************
 * qty__<A> — スカラ量ホルダ（原典 backup/tmp_include/qty.h）
 *
 * codegen 双子（qty__ / base_qty__ / num__ / type__）の一つ。メンバ名と
 * アクセサ名（is_qty / qty_is / qtying / check_qty）だけが異なる同型で、
 * 「メンバ名＝役割名」の原典規約を保存するため個別ファイルで移植する
 * （呼び出し面の名前が仕様。CRTP への畳み込みは行わない）。
 *
 * element__ の D 基底（要素数の台帳）として使われる。演算は
 * operator_base のスカラ総称へ委譲する。
 *
 * 原典からの現代化（4双子とも共通）:
 *   - throw(std::bad_range) 動的例外仕様の除去、__attribute__((flatten)) 非採用
 *   - operator=(const A*) が「ポインタ値をそのまま代入」だった原典バグを
 *     修正（ctor(const A*) は逆参照しており、逆参照が明白な意図）
 *   - operator==/!= の到達しない `long ret;` を除去
 *   - CHECK_LIMIT(_ref, qty) マクロ（include/pre.h:62）は式で展開
 *     （0 <= _ref && _ref <= qty。上限は閉区間＝原典どおり）
 *********************************************/
#include "../types.h"
#include "operator_base.h"

template <typename A>
class qty__ {
 protected:
	A qty;
 public:
	qty__(const A& A_qty = 0) : qty(A_qty){}
	qty__(const A* A_qty) : qty(*A_qty){}
	qty__(const qty__<A>& _qty) : qty(_qty.qty){}
	qty__(const qty__<A>* _qty) : qty(_qty->qty){}

	/* --- 比較（vs 生値） --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(qty, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(qty, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(qty, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(qty, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(qty, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(qty, _ref); }

	/* --- 算術（vs 生値）: clone してから複合代入（原典どおり） --- */
	qty__<A> operator+(const A& _ref){ qty__<A> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	qty__<A> operator-(const A& _ref){ qty__<A> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	qty__<A> operator/(const A& _ref){ qty__<A> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	qty__<A> operator*(const A& _ref){ qty__<A> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	qty__<A>& operator=(const A& _ref){ this->qty = _ref; return *this; }
	qty__<A>& operator=(const A* _ref){ this->qty = *_ref; return *this; }   /* 修正: 原典はポインタ代入 */
	qty__<A>& operator+=(const A& _ref){ arith_add_assign<A>(qty, _ref); return *this; }
	qty__<A>& operator-=(const A& _ref){ arith_sub_assign<A>(qty, _ref); return *this; }
	qty__<A>& operator/=(const A& _ref){ arith_div_assign<A>(qty, _ref); return *this; }
	qty__<A>& operator*=(const A& _ref){ arith_mul_assign<A>(qty, _ref); return *this; }

	/* --- 比較（vs qty__）: 同値のとき 0 を返す原典の三分律 --- */
	result_ operator> (const qty__<A>& _ref){ if ( qty != _ref.qty ) return comp_gt<A>(qty, _ref.qty); return 0; }
	result_ operator< (const qty__<A>& _ref){ if ( qty != _ref.qty ) return comp_lt<A>(qty, _ref.qty); return 0; }
	result_ operator>=(const qty__<A>& _ref){ return comp_ge<A>(qty, _ref.qty); }
	result_ operator<=(const qty__<A>& _ref){ return comp_le<A>(qty, _ref.qty); }
	result_ operator==(const qty__<A>& _ref){ return comp_eq<A>(qty, _ref.qty); }
	result_ operator!=(const qty__<A>& _ref){ return comp_ne<A>(qty, _ref.qty); }

	qty__<A> operator+(const qty__<A>& _ref){ qty__<A> tmp(this->clone()); arith_add_assign<A>(tmp.qty, _ref.qty); return tmp; }
	qty__<A> operator-(const qty__<A>& _ref){ qty__<A> tmp(this->clone()); arith_sub_assign<A>(tmp.qty, _ref.qty); return tmp; }
	qty__<A> operator/(const qty__<A>& _ref){ qty__<A> tmp(this->clone()); arith_div_assign<A>(tmp.qty, _ref.qty); return tmp; }
	qty__<A> operator*(const qty__<A>& _ref){ qty__<A> tmp(this->clone()); arith_mul_assign<A>(tmp.qty, _ref.qty); return tmp; }

	qty__<A>& operator=(const qty__<A>& _ref){ this->qty = _ref.qty; return *this; }
	qty__<A>& operator=(const qty__<A>* _ref){ this->qty = _ref->qty; return *this; }
	qty__<A>& operator+=(const qty__<A>& _ref){ arith_add_assign<A>(qty, _ref.qty); return *this; }
	qty__<A>& operator-=(const qty__<A>& _ref){ arith_sub_assign<A>(qty, _ref.qty); return *this; }
	qty__<A>& operator/=(const qty__<A>& _ref){ arith_div_assign<A>(qty, _ref.qty); return *this; }
	qty__<A>& operator*=(const qty__<A>& _ref){ arith_mul_assign<A>(qty, _ref.qty); return *this; }

	/* --- アクセス・複製・状態 --- */
	A& operator()() const { return (A&)this->qty; }   /* 原典契約（const から可変参照） */
	qty__<A> clone(){ qty__<A> tmp; tmp.qty = this->qty; return tmp; }
	qty__<A>* clonep(){ qty__<A>* tmp = new qty__<A>; tmp->qty = this->qty; return tmp; }
	bool check_qty(idx_ _ref){ return 0 <= _ref && _ref <= qty; }   /* CHECK_LIMIT */
	void clear(){ this->qty = 0; }
	A is_qty() const { return qty; }
	void qty_is(A _qty){ this->qty = _qty; }
	A qtying(A _ref){
		A ret;
		if ( _ref > qty ) ret = qty;
		else if ( _ref < 0 ) ret = 0;
		else ret = _ref;
		return ret;
	}
};

#endif /* CPP_DEVEL_UNI_QTY_H */

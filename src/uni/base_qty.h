#ifndef CPP_DEVEL_UNI_BASE_QTY_H
#define CPP_DEVEL_UNI_BASE_QTY_H
/*********************************************
 * base_qty__<A> — スカラ量ホルダ（原典 backup/tmp_include/base_qty.h）
 *
 * codegen 双子（qty__ / base_qty__ / num__ / type__）の一つ。qty.h と
 * メンバ名・アクセサ名のみが異なる同型（原典スナップショットに対して
 * 置換一致を確認済み）。設計注記と原典からの修正点は qty.h 冒頭を参照。
 * 本ファイルは qty.h から機械置換で生成し、双子性を保証している。
 *********************************************/
#include "../types.h"
#include "operator_base.h"

template <typename A>
class base_qty__ {
 protected:
	A base_qty;
 public:
	base_qty__(const A& A_base_qty = 0) : base_qty(A_base_qty){}
	base_qty__(const A* A_base_qty) : base_qty(*A_base_qty){}
	base_qty__(const base_qty__<A>& _base_qty) : base_qty(_base_qty.base_qty){}
	base_qty__(const base_qty__<A>* _base_qty) : base_qty(_base_qty->base_qty){}

	/* --- 比較（vs 生値） --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(base_qty, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(base_qty, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(base_qty, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(base_qty, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(base_qty, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(base_qty, _ref); }

	/* --- 算術（vs 生値）: clone してから複合代入（原典どおり） --- */
	base_qty__<A> operator+(const A& _ref){ base_qty__<A> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	base_qty__<A> operator-(const A& _ref){ base_qty__<A> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	base_qty__<A> operator/(const A& _ref){ base_qty__<A> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	base_qty__<A> operator*(const A& _ref){ base_qty__<A> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	base_qty__<A>& operator=(const A& _ref){ this->base_qty = _ref; return *this; }
	base_qty__<A>& operator=(const A* _ref){ this->base_qty = *_ref; return *this; }   /* 修正: 原典はポインタ代入 */
	base_qty__<A>& operator+=(const A& _ref){ arith_add_assign<A>(base_qty, _ref); return *this; }
	base_qty__<A>& operator-=(const A& _ref){ arith_sub_assign<A>(base_qty, _ref); return *this; }
	base_qty__<A>& operator/=(const A& _ref){ arith_div_assign<A>(base_qty, _ref); return *this; }
	base_qty__<A>& operator*=(const A& _ref){ arith_mul_assign<A>(base_qty, _ref); return *this; }

	/* --- 比較（vs base_qty__）: 同値のとき 0 を返す原典の三分律 --- */
	result_ operator> (const base_qty__<A>& _ref){ if ( base_qty != _ref.base_qty ) return comp_gt<A>(base_qty, _ref.base_qty); return 0; }
	result_ operator< (const base_qty__<A>& _ref){ if ( base_qty != _ref.base_qty ) return comp_lt<A>(base_qty, _ref.base_qty); return 0; }
	result_ operator>=(const base_qty__<A>& _ref){ return comp_ge<A>(base_qty, _ref.base_qty); }
	result_ operator<=(const base_qty__<A>& _ref){ return comp_le<A>(base_qty, _ref.base_qty); }
	result_ operator==(const base_qty__<A>& _ref){ return comp_eq<A>(base_qty, _ref.base_qty); }
	result_ operator!=(const base_qty__<A>& _ref){ return comp_ne<A>(base_qty, _ref.base_qty); }

	base_qty__<A> operator+(const base_qty__<A>& _ref){ base_qty__<A> tmp(this->clone()); arith_add_assign<A>(tmp.base_qty, _ref.base_qty); return tmp; }
	base_qty__<A> operator-(const base_qty__<A>& _ref){ base_qty__<A> tmp(this->clone()); arith_sub_assign<A>(tmp.base_qty, _ref.base_qty); return tmp; }
	base_qty__<A> operator/(const base_qty__<A>& _ref){ base_qty__<A> tmp(this->clone()); arith_div_assign<A>(tmp.base_qty, _ref.base_qty); return tmp; }
	base_qty__<A> operator*(const base_qty__<A>& _ref){ base_qty__<A> tmp(this->clone()); arith_mul_assign<A>(tmp.base_qty, _ref.base_qty); return tmp; }

	base_qty__<A>& operator=(const base_qty__<A>& _ref){ this->base_qty = _ref.base_qty; return *this; }
	base_qty__<A>& operator=(const base_qty__<A>* _ref){ this->base_qty = _ref->base_qty; return *this; }
	base_qty__<A>& operator+=(const base_qty__<A>& _ref){ arith_add_assign<A>(base_qty, _ref.base_qty); return *this; }
	base_qty__<A>& operator-=(const base_qty__<A>& _ref){ arith_sub_assign<A>(base_qty, _ref.base_qty); return *this; }
	base_qty__<A>& operator/=(const base_qty__<A>& _ref){ arith_div_assign<A>(base_qty, _ref.base_qty); return *this; }
	base_qty__<A>& operator*=(const base_qty__<A>& _ref){ arith_mul_assign<A>(base_qty, _ref.base_qty); return *this; }

	/* --- アクセス・複製・状態 --- */
	A& operator()() const { return (A&)this->base_qty; }   /* 原典契約（const から可変参照） */
	base_qty__<A> clone(){ base_qty__<A> tmp; tmp.base_qty = this->base_qty; return tmp; }
	base_qty__<A>* clonep(){ base_qty__<A>* tmp = new base_qty__<A>; tmp->base_qty = this->base_qty; return tmp; }
	bool check_base_qty(idx_ _ref){ return 0 <= _ref && _ref <= base_qty; }   /* CHECK_LIMIT */
	void clear(){ this->base_qty = 0; }
	A is_base_qty() const { return base_qty; }
	void base_qty_is(A _base_qty){ this->base_qty = _base_qty; }
	A base_qtying(A _ref){
		A ret;
		if ( _ref > base_qty ) ret = base_qty;
		else if ( _ref < 0 ) ret = 0;
		else ret = _ref;
		return ret;
	}
};

#endif /* CPP_DEVEL_UNI_BASE_QTY_H */

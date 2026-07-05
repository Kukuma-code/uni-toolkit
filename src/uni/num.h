#ifndef CPP_DEVEL_UNI_NUM_H
#define CPP_DEVEL_UNI_NUM_H
/*********************************************
 * num__<A> — スカラ量ホルダ（原典 backup/tmp_include/num.h）
 *
 * codegen 双子（qty__ / base_qty__ / num__ / type__）の一つ。qty.h と
 * メンバ名・アクセサ名のみが異なる同型（原典スナップショットに対して
 * 置換一致を確認済み）。設計注記と原典からの修正点は qty.h 冒頭を参照。
 * 本ファイルは qty.h から機械置換で生成し、双子性を保証している。
 *********************************************/
#include "../types.h"
#include "operator_base.h"

template <typename A>
class num__ {
 protected:
	A num;
 public:
	num__(const A& A_num = 0) : num(A_num){}
	num__(const A* A_num) : num(*A_num){}
	num__(const num__<A>& _num) : num(_num.num){}
	num__(const num__<A>* _num) : num(_num->num){}

	/* --- 比較（vs 生値） --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(num, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(num, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(num, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(num, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(num, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(num, _ref); }

	/* --- 算術（vs 生値）: clone してから複合代入（原典どおり） --- */
	num__<A> operator+(const A& _ref){ num__<A> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	num__<A> operator-(const A& _ref){ num__<A> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	num__<A> operator/(const A& _ref){ num__<A> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	num__<A> operator*(const A& _ref){ num__<A> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	num__<A>& operator=(const A& _ref){ this->num = _ref; return *this; }
	num__<A>& operator=(const A* _ref){ this->num = *_ref; return *this; }   /* 修正: 原典はポインタ代入 */
	num__<A>& operator+=(const A& _ref){ arith_add_assign<A>(num, _ref); return *this; }
	num__<A>& operator-=(const A& _ref){ arith_sub_assign<A>(num, _ref); return *this; }
	num__<A>& operator/=(const A& _ref){ arith_div_assign<A>(num, _ref); return *this; }
	num__<A>& operator*=(const A& _ref){ arith_mul_assign<A>(num, _ref); return *this; }

	/* --- 比較（vs num__）: 同値のとき 0 を返す原典の三分律 --- */
	result_ operator> (const num__<A>& _ref){ if ( num != _ref.num ) return comp_gt<A>(num, _ref.num); return 0; }
	result_ operator< (const num__<A>& _ref){ if ( num != _ref.num ) return comp_lt<A>(num, _ref.num); return 0; }
	result_ operator>=(const num__<A>& _ref){ return comp_ge<A>(num, _ref.num); }
	result_ operator<=(const num__<A>& _ref){ return comp_le<A>(num, _ref.num); }
	result_ operator==(const num__<A>& _ref){ return comp_eq<A>(num, _ref.num); }
	result_ operator!=(const num__<A>& _ref){ return comp_ne<A>(num, _ref.num); }

	num__<A> operator+(const num__<A>& _ref){ num__<A> tmp(this->clone()); arith_add_assign<A>(tmp.num, _ref.num); return tmp; }
	num__<A> operator-(const num__<A>& _ref){ num__<A> tmp(this->clone()); arith_sub_assign<A>(tmp.num, _ref.num); return tmp; }
	num__<A> operator/(const num__<A>& _ref){ num__<A> tmp(this->clone()); arith_div_assign<A>(tmp.num, _ref.num); return tmp; }
	num__<A> operator*(const num__<A>& _ref){ num__<A> tmp(this->clone()); arith_mul_assign<A>(tmp.num, _ref.num); return tmp; }

	num__<A>& operator=(const num__<A>& _ref){ this->num = _ref.num; return *this; }
	num__<A>& operator=(const num__<A>* _ref){ this->num = _ref->num; return *this; }
	num__<A>& operator+=(const num__<A>& _ref){ arith_add_assign<A>(num, _ref.num); return *this; }
	num__<A>& operator-=(const num__<A>& _ref){ arith_sub_assign<A>(num, _ref.num); return *this; }
	num__<A>& operator/=(const num__<A>& _ref){ arith_div_assign<A>(num, _ref.num); return *this; }
	num__<A>& operator*=(const num__<A>& _ref){ arith_mul_assign<A>(num, _ref.num); return *this; }

	/* --- アクセス・複製・状態 --- */
	A& operator()() const { return (A&)this->num; }   /* 原典契約（const から可変参照） */
	num__<A> clone(){ num__<A> tmp; tmp.num = this->num; return tmp; }
	num__<A>* clonep(){ num__<A>* tmp = new num__<A>; tmp->num = this->num; return tmp; }
	bool check_num(idx_ _ref){ return 0 <= _ref && _ref <= num; }   /* CHECK_LIMIT */
	void clear(){ this->num = 0; }
	A is_num() const { return num; }
	void num_is(A _num){ this->num = _num; }
	A numing(A _ref){
		A ret;
		if ( _ref > num ) ret = num;
		else if ( _ref < 0 ) ret = 0;
		else ret = _ref;
		return ret;
	}
};

#endif /* CPP_DEVEL_UNI_NUM_H */

#ifndef CPP_DEVEL_UNI_TYPE_H
#define CPP_DEVEL_UNI_TYPE_H
/*********************************************
 * type__<A> — スカラ量ホルダ（原典 backup/tmp_include/type.h）
 *
 * codegen 双子（qty__ / base_qty__ / num__ / type__）の一つ。qty.h と
 * メンバ名・アクセサ名のみが異なる同型（原典スナップショットに対して
 * 置換一致を確認済み）。設計注記と原典からの修正点は qty.h 冒頭を参照。
 * 本ファイルは qty.h から機械置換で生成し、双子性を保証している。
 *********************************************/
#include "../types.h"
#include "operator_base.h"

template <typename A>
class type__ {
 protected:
	A type;
 public:
	type__(const A& A_type = 0) : type(A_type){}
	type__(const A* A_type) : type(*A_type){}
	type__(const type__<A>& _type) : type(_type.type){}
	type__(const type__<A>* _type) : type(_type->type){}

	/* --- 比較（vs 生値） --- */
	result_ operator> (const A& _ref){ return comp_gt<A>(type, _ref); }
	result_ operator< (const A& _ref){ return comp_lt<A>(type, _ref); }
	result_ operator>=(const A& _ref){ return comp_ge<A>(type, _ref); }
	result_ operator<=(const A& _ref){ return comp_le<A>(type, _ref); }
	result_ operator==(const A& _ref){ return comp_eq<A>(type, _ref); }
	result_ operator!=(const A& _ref){ return comp_ne<A>(type, _ref); }

	/* --- 算術（vs 生値）: clone してから複合代入（原典どおり） --- */
	type__<A> operator+(const A& _ref){ type__<A> tmp(this->clone()); tmp.operator+=(_ref); return tmp; }
	type__<A> operator-(const A& _ref){ type__<A> tmp(this->clone()); tmp.operator-=(_ref); return tmp; }
	type__<A> operator/(const A& _ref){ type__<A> tmp(this->clone()); tmp.operator/=(_ref); return tmp; }
	type__<A> operator*(const A& _ref){ type__<A> tmp(this->clone()); tmp.operator*=(_ref); return tmp; }

	type__<A>& operator=(const A& _ref){ this->type = _ref; return *this; }
	type__<A>& operator=(const A* _ref){ this->type = *_ref; return *this; }   /* 修正: 原典はポインタ代入 */
	type__<A>& operator+=(const A& _ref){ arith_add_assign<A>(type, _ref); return *this; }
	type__<A>& operator-=(const A& _ref){ arith_sub_assign<A>(type, _ref); return *this; }
	type__<A>& operator/=(const A& _ref){ arith_div_assign<A>(type, _ref); return *this; }
	type__<A>& operator*=(const A& _ref){ arith_mul_assign<A>(type, _ref); return *this; }

	/* --- 比較（vs type__）: 同値のとき 0 を返す原典の三分律 --- */
	result_ operator> (const type__<A>& _ref){ if ( type != _ref.type ) return comp_gt<A>(type, _ref.type); return 0; }
	result_ operator< (const type__<A>& _ref){ if ( type != _ref.type ) return comp_lt<A>(type, _ref.type); return 0; }
	result_ operator>=(const type__<A>& _ref){ return comp_ge<A>(type, _ref.type); }
	result_ operator<=(const type__<A>& _ref){ return comp_le<A>(type, _ref.type); }
	result_ operator==(const type__<A>& _ref){ return comp_eq<A>(type, _ref.type); }
	result_ operator!=(const type__<A>& _ref){ return comp_ne<A>(type, _ref.type); }

	type__<A> operator+(const type__<A>& _ref){ type__<A> tmp(this->clone()); arith_add_assign<A>(tmp.type, _ref.type); return tmp; }
	type__<A> operator-(const type__<A>& _ref){ type__<A> tmp(this->clone()); arith_sub_assign<A>(tmp.type, _ref.type); return tmp; }
	type__<A> operator/(const type__<A>& _ref){ type__<A> tmp(this->clone()); arith_div_assign<A>(tmp.type, _ref.type); return tmp; }
	type__<A> operator*(const type__<A>& _ref){ type__<A> tmp(this->clone()); arith_mul_assign<A>(tmp.type, _ref.type); return tmp; }

	type__<A>& operator=(const type__<A>& _ref){ this->type = _ref.type; return *this; }
	type__<A>& operator=(const type__<A>* _ref){ this->type = _ref->type; return *this; }
	type__<A>& operator+=(const type__<A>& _ref){ arith_add_assign<A>(type, _ref.type); return *this; }
	type__<A>& operator-=(const type__<A>& _ref){ arith_sub_assign<A>(type, _ref.type); return *this; }
	type__<A>& operator/=(const type__<A>& _ref){ arith_div_assign<A>(type, _ref.type); return *this; }
	type__<A>& operator*=(const type__<A>& _ref){ arith_mul_assign<A>(type, _ref.type); return *this; }

	/* --- アクセス・複製・状態 --- */
	A& operator()() const { return (A&)this->type; }   /* 原典契約（const から可変参照） */
	type__<A> clone(){ type__<A> tmp; tmp.type = this->type; return tmp; }
	type__<A>* clonep(){ type__<A>* tmp = new type__<A>; tmp->type = this->type; return tmp; }
	bool check_type(idx_ _ref){ return 0 <= _ref && _ref <= type; }   /* CHECK_LIMIT */
	void clear(){ this->type = 0; }
	A is_type() const { return type; }
	void type_is(A _type){ this->type = _type; }
	A typeing(A _ref){
		A ret;
		if ( _ref > type ) ret = type;
		else if ( _ref < 0 ) ret = 0;
		else ret = _ref;
		return ret;
	}
};

#endif /* CPP_DEVEL_UNI_TYPE_H */

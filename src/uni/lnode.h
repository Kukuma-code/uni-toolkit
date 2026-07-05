#ifndef CPP_DEVEL_UNI_LNODE_H
#define CPP_DEVEL_UNI_LNODE_H
/*********************************************
 * lnode__<A,B,C> — リスト要素（原典 backup/tmp_include/lnode.h）
 *
 *   lnode__ : public C, public lhdr__<lnode__<A,B,C>>
 *   C = uni__<A,B>（既定）= 値部、lhdr__ = prev/next の連結部。
 *
 * 演算・アクセスはすべて C へ委譲する転送層。lnode__ どうしの代入と
 * clone/clonep のみ lhdr 部（prev/next）も複写する（copy ctor も同様）。
 * 算術の複合代入（vs lnode__）は C 部のみに作用する（原典どおり）。
 *
 * 原典からの現代化:
 *   - throw(std::bad_range) 仕様除去、INLINE / flatten 非採用
 *   - コメントアウト済みの旧 ctor 群（qty_ 版・ポインタ版・C 参照版の
 *     重複宣言）は移植しない
 *   - info() の %lx キャストは %p / void* に置換（体裁のみ）
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "uni.h"
#include "lhdr.h"

template <typename A, typename B = A*, typename C = uni__<A, B>>
class lnode__ : public C, public lhdr__<lnode__<A, B, C>> {
 public:
	lnode__(const A* C21_node) : C(C21_node), lhdr__<lnode__<A, B, C>>(){}
	lnode__(const A* C21_node, const qty_ C21_node_qty) : C(C21_node, C21_node_qty), lhdr__<lnode__<A, B, C>>(){}
	lnode__(const C& _C) : C(_C), lhdr__<lnode__<A, B, C>>(){}
	lnode__(const C* _C) : C(_C), lhdr__<lnode__<A, B, C>>(){}
	lnode__(const qty_ C21_node_qty = 0) : C(C21_node_qty), lhdr__<lnode__<A, B, C>>(){}
	lnode__(const lnode__<A, B, C>& _lnode) : C(_lnode), lhdr__<lnode__<A, B, C>>(_lnode){}
	lnode__(const lnode__<A, B, C>* _lnode) : C(_lnode), lhdr__<lnode__<A, B, C>>(_lnode){}
	~lnode__(){}

	/* --- 比較・算術（vs 生値）: C へ転送 --- */
	result_ operator> (const A& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const A& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const A& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const A& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const A& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const A& _ref){ return this->C::operator!=(_ref); }
	lnode__<A, B, C> operator+(const A& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	lnode__<A, B, C> operator-(const A& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator-=(_ref); return tmp; }
	lnode__<A, B, C> operator/(const A& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator/=(_ref); return tmp; }
	lnode__<A, B, C> operator*(const A& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator*=(_ref); return tmp; }
	lnode__<A, B, C>& operator=(const A& _ref){ this->C::operator=(_ref); return *this; }
	lnode__<A, B, C>& operator=(const A* _ref){ this->C::operator=(_ref); return *this; }
	lnode__<A, B, C>& operator+=(const A& _ref){ this->C::operator+=(_ref); return *this; }
	lnode__<A, B, C>& operator-=(const A& _ref){ this->C::operator-=(_ref); return *this; }
	lnode__<A, B, C>& operator/=(const A& _ref){ this->C::operator/=(_ref); return *this; }
	lnode__<A, B, C>& operator*=(const A& _ref){ this->C::operator*=(_ref); return *this; }

	/* --- 比較・算術（vs lnode__）: C へ転送（代入のみ lhdr 部も複写） --- */
	result_ operator> (const lnode__<A, B, C>& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const lnode__<A, B, C>& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const lnode__<A, B, C>& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const lnode__<A, B, C>& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const lnode__<A, B, C>& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const lnode__<A, B, C>& _ref){ return this->C::operator!=(_ref); }
	lnode__<A, B, C> operator+(const lnode__<A, B, C>& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	lnode__<A, B, C> operator-(const lnode__<A, B, C>& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator-=(_ref); return tmp; }
	lnode__<A, B, C> operator/(const lnode__<A, B, C>& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator/=(_ref); return tmp; }
	lnode__<A, B, C> operator*(const lnode__<A, B, C>& _ref){ lnode__<A, B, C> tmp(this->clone()); tmp.C::operator*=(_ref); return tmp; }
	lnode__<A, B, C>& operator=(const lnode__<A, B, C>& _ref){
		this->C::operator=(_ref); this->lhdr__<lnode__<A, B, C>>::operator=(_ref); return *this;
	}
	lnode__<A, B, C>& operator=(const lnode__<A, B, C>* _ref){
		this->C::operator=(_ref); this->lhdr__<lnode__<A, B, C>>::operator=(_ref); return *this;
	}
	lnode__<A, B, C>& operator+=(const lnode__<A, B, C>& _ref){ this->C::operator+=(_ref); return *this; }
	lnode__<A, B, C>& operator-=(const lnode__<A, B, C>& _ref){ this->C::operator-=(_ref); return *this; }
	lnode__<A, B, C>& operator/=(const lnode__<A, B, C>& _ref){ this->C::operator/=(_ref); return *this; }
	lnode__<A, B, C>& operator*=(const lnode__<A, B, C>& _ref){ this->C::operator*=(_ref); return *this; }

	/* --- 比較・算術（vs C） --- */
	result_ operator> (const C& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const C& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const C& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const C& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const C& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const C& _ref){ return this->C::operator!=(_ref); }
	lnode__<A, B, C> operator+(const C& _ref){ lnode__<A, B, C> tmp(this); tmp.C::operator+=(_ref); return tmp; }
	lnode__<A, B, C> operator-(const C& _ref){ lnode__<A, B, C> tmp(this); tmp.C::operator-=(_ref); return tmp; }
	lnode__<A, B, C> operator/(const C& _ref){ lnode__<A, B, C> tmp(this); tmp.C::operator/=(_ref); return tmp; }
	lnode__<A, B, C> operator*(const C& _ref){ lnode__<A, B, C> tmp(this); tmp.C::operator*=(_ref); return tmp; }
	lnode__<A, B, C>& operator=(const C& _ref){ this->C::operator=(_ref); return *this; }
	lnode__<A, B, C>& operator=(const C* _ref){ this->C::operator=(_ref); return *this; }
	lnode__<A, B, C>& operator+=(const C& _ref){ this->C::operator+=(_ref); return *this; }
	lnode__<A, B, C>& operator-=(const C& _ref){ this->C::operator-=(_ref); return *this; }
	lnode__<A, B, C>& operator/=(const C& _ref){ this->C::operator/=(_ref); return *this; }
	lnode__<A, B, C>& operator*=(const C& _ref){ this->C::operator*=(_ref); return *this; }

	/* --- アクセス・複製・状態 --- */
	B& operator()() const { return this->C::operator()(); }
	A& operator()(const idx_ _idx){ return this->C::operator()(_idx); }
	A& operator[](const idx_ _idx){ return this->C::operator()(_idx); }
	lnode__<A, B, C> clone(){
		lnode__<A, B, C> tmp;
		tmp.C::operator=(this->C::clone());
		tmp.lhdr__<lnode__<A, B, C>>::operator=(this->lhdr__<lnode__<A, B, C>>::clone());
		return tmp;
	}
	lnode__<A, B, C>* clonep(){
		lnode__<A, B, C>* tmp = new lnode__<A, B, C>;
		tmp->C::operator=(this->C::clone());
		tmp->lhdr__<lnode__<A, B, C>>::operator=(this->lhdr__<lnode__<A, B, C>>::clone());
		return tmp;
	}
	void clear(){ this->C::clear(); }
	lnode__<A, B, C>* is_next(){ return this->next; }
	lnode__<A, B, C>* is_prev(){ return this->prev; }
	void info(){
		printf("<lnode__><this node:%p> <prev:%p> <next:%p> ",
		       (void*)this, (void*)this->prev, (void*)this->next);
		this->C::info();
	}
};

/* クラス全体の出力（原典 lnode.h:203） */
template <typename A, typename B, typename C>
inline void p(const lnode__<A, B, C>& _ref, status_ _status = 0){
	((lnode__<A, B, C>&)_ref).p();
	newline(_status);
}

#endif /* CPP_DEVEL_UNI_LNODE_H */

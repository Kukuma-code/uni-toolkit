#ifndef CPP_DEVEL_UNI_ALLOCATOR_ELEMENT_H
#define CPP_DEVEL_UNI_ALLOCATOR_ELEMENT_H
/*********************************************
 * allocator_element__<A,B,C,D> — ノード表＋容量台帳の合成
 * （原典 backup/tmp_include/allocator_element.h）
 *
 *   allocator_element__ : public C, public D
 *   C = allocator_node__<A,B>（既定）= ノード表、
 *   D = base_qty__<qty_>（既定）= 「基底容量」の台帳。
 *   element__（uni__ + qty__）の codegen 双子で、委譲規約は element.h の
 *   冒頭注記と同一（比較は C のみ / 複合代入は C+D / 代入は C 後に D 更新）。
 *   ballocator__ の B 基底として使われる（スロット数 = is_base_qty()）。
 *
 * 原典からの現代化は element.h と同一方針。ポインタ引数の operator+ /
 * operator+= は原典どおり宣言のみ（使用時リンクエラー）。
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "base_qty.h"
#include "allocator_node.h"

template <typename A, typename B = A*, typename C = allocator_node__<A, B>, typename D = base_qty__<qty_>>
class allocator_element__ : public C, public D {
 public:
	allocator_element__(const A* C_node) : C(C_node), D(this->C::is_qty()){}
	allocator_element__(const A* C21_node, const qty_& B_qty) : C(C21_node, B_qty), D(B_qty){}
	allocator_element__(const A* C21_node, const qty_* B_qty) : C(C21_node, B_qty), D(B_qty){}
	allocator_element__(const C& _C) : C(_C), D(this->C::is_qty()){}
	allocator_element__(const C& _C, const qty_& B_qty) : C(_C, B_qty), D(B_qty){}
	allocator_element__(const C& _C, const qty_* B_qty) : C(_C, B_qty), D(B_qty){}
	allocator_element__(const C* _C, const qty_& B_qty) : C(_C, B_qty), D(B_qty){}
	allocator_element__(const C* _C, const qty_* B_qty) : C(_C, B_qty), D(B_qty){}
	allocator_element__(const qty_ _qty = 0) : C(_qty), D(_qty){}
	allocator_element__(const allocator_element__<A, B, C, D>& _allocator_element) : C(_allocator_element), D(_allocator_element){}
	allocator_element__(const allocator_element__<A, B, C, D>* _allocator_element) : C(_allocator_element), D(_allocator_element){}
	~allocator_element__(){}

	/* --- 比較・算術（vs 生値） --- */
	result_ operator> (const A& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const A& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const A& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const A& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const A& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const A& _ref){ return this->C::operator!=(_ref); }
	allocator_element__<A, B, C, D> operator+(const A& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator-(const A& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator-=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator/(const A& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator/=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator*(const A& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator*=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator+(const A* _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	allocator_element__<A, B, C, D>& operator=(const A& _ref){ this->C::operator=(_ref); this->D::operator=(::is_qty(_ref)); return *this; }
	allocator_element__<A, B, C, D>& operator=(const A* _ref){ this->C::operator=(_ref); this->D::operator=(::is_qty(_ref)); return *this; }
	allocator_element__<A, B, C, D>& operator+=(const A& _ref){ this->C::operator+=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator-=(const A& _ref){ this->C::operator-=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator/=(const A& _ref){ this->C::operator/=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator*=(const A& _ref){ this->C::operator*=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator+=(const A* _ref){ this->C::operator+=(_ref); this->D::operator+=(_ref); return *this; }

	/* --- 比較・算術（vs allocator_element__） --- */
	result_ operator> (const allocator_element__<A, B, C, D>& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const allocator_element__<A, B, C, D>& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const allocator_element__<A, B, C, D>& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const allocator_element__<A, B, C, D>& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const allocator_element__<A, B, C, D>& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const allocator_element__<A, B, C, D>& _ref){ return this->C::operator!=(_ref); }
	allocator_element__<A, B, C, D> operator+(const allocator_element__<A, B, C, D>& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); tmp.D::operator+=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator-(const allocator_element__<A, B, C, D>& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator-=(_ref); tmp.D::operator-=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator/(const allocator_element__<A, B, C, D>& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator/=(_ref); tmp.D::operator/=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator*(const allocator_element__<A, B, C, D>& _ref){ allocator_element__<A, B, C, D> tmp(this->clone()); tmp.C::operator*=(_ref); tmp.D::operator*=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator+(const allocator_element__<A, B, C, D>* _ref);   /* 原典: 宣言のみ（定義なし。使用時リンクエラー） */
	allocator_element__<A, B, C, D>& operator=(const allocator_element__<A, B, C, D>& _ref){ this->C::operator=(_ref); this->D::operator=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator=(const allocator_element__<A, B, C, D>* _ref){ this->C::operator=(_ref); this->D::operator=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator+=(const allocator_element__<A, B, C, D>& _ref){ this->C::operator+=(_ref); this->D::operator+=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator-=(const allocator_element__<A, B, C, D>& _ref){ this->C::operator-=(_ref); this->D::operator-=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator/=(const allocator_element__<A, B, C, D>& _ref){ this->C::operator/=(_ref); this->D::operator/=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator*=(const allocator_element__<A, B, C, D>& _ref){ this->C::operator*=(_ref); this->D::operator*=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator+=(const allocator_element__<A, B, C, D>* _ref);   /* 原典: 宣言のみ（定義なし。使用時リンクエラー） */

	/* --- 比較・算術（vs C） --- */
	result_ operator> (const C& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const C& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const C& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const C& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const C& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const C& _ref){ return this->C::operator!=(_ref); }
	allocator_element__<A, B, C, D> operator+(const C& _ref){ allocator_element__<A, B, C, D> tmp(this); tmp.C::operator+=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator-(const C& _ref){ allocator_element__<A, B, C, D> tmp(this); tmp.C::operator-=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator/(const C& _ref){ allocator_element__<A, B, C, D> tmp(this); tmp.C::operator/=(_ref); return tmp; }
	allocator_element__<A, B, C, D> operator*(const C& _ref){ allocator_element__<A, B, C, D> tmp(this); tmp.C::operator*=(_ref); return tmp; }
	allocator_element__<A, B, C, D>& operator=(const C& _ref){ this->C::operator=(_ref); this->D::operator=(_ref.is_qty()); return *this; }
	allocator_element__<A, B, C, D>& operator=(const C* _ref){ this->C::operator=(_ref); this->D::operator=(_ref->is_qty()); return *this; }
	allocator_element__<A, B, C, D>& operator+=(const C& _ref){ this->C::operator+=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator-=(const C& _ref){ this->C::operator-=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator/=(const C& _ref){ this->C::operator/=(_ref); return *this; }
	allocator_element__<A, B, C, D>& operator*=(const C& _ref){ this->C::operator*=(_ref); return *this; }

	/* --- アクセス・複製 --- */
	B& operator()() const { return this->C::operator()(); }
	A& operator()(const idx_ _idx){ return this->C::operator()(_idx); }
	A& operator[](const idx_ _idx){ return this->C::operator()(_idx); }
	allocator_element__<A, B, C, D> clone(){
		allocator_element__<A, B, C, D> tmp;
		tmp.C::operator=(this->C::clone());
		tmp.D::operator=(this->D::clone());
		return tmp;
	}
	allocator_element__<A, B, C, D>* clonep(){
		allocator_element__<A, B, C, D>* tmp = new allocator_element__<A, B, C, D>;
		tmp->C::operator=(this->C::clone());
		tmp->D::operator=(this->D::clone());
		return tmp;
	}

	/* --- 状態・出力 --- */
	void clear(){ this->C::clear(); this->D::clear(); }
	void info(status_ _status = 0){
		(void)_status;   /* 原典から不使用 */
		this->C::info((status_)0);
		printf("<allocator_element__>base_qty:%ld\n", (long)this->is_base_qty());
	}
	using D::is_base_qty;
	using D::check_base_qty;
	using D::base_qtying;
	using C::p;
};

/* クラス全体の出力（原典 allocator_element.h:248） */
template <typename A, typename B, typename C, typename D>
inline void p(const allocator_element__<A, B, C, D>& _ref, status_ _status = 0){
	((allocator_element__<A, B, C, D>&)_ref).p();
	newline(_status);
}

/* 原典は element.h と同じ HAS_ELEM_STRING ガードで estring_ 系 typedef を
   二重定義していた（include 順の早い方が勝つ）。再構築では typedef の
   実体は element.h 側に一本化し、こちらは原典の痕跡として記録のみ残す */

#endif /* CPP_DEVEL_UNI_ALLOCATOR_ELEMENT_H */

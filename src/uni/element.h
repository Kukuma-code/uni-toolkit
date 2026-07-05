#ifndef CPP_DEVEL_UNI_ELEMENT_H
#define CPP_DEVEL_UNI_ELEMENT_H
/*********************************************
 * element__<A,B,C,D> — 値部＋数量台帳の合成（原典 backup/tmp_include/element.h）
 *
 *   element__ : public C, public D
 *   C = uni__<A,B>（既定）= 値部、D = qty__<qty_>（既定）= 「使用数」の台帳。
 *   uni__ の is_qty が「確保数」を返すのに対し、element__ の is_qty
 *   （using D::is_qty）は台帳の値を返す。確保数は this->C::is_qty() で参照する。
 *
 * 委譲規約（原典どおり保存）:
 *   - 比較は C のみ（D は関与しない）
 *   - 複合代入 += -= 等（vs element__）は C と D の両方に作用
 *   - 代入 =（vs 生値/C）は C に代入後、D を新しい数量で更新
 *   - operator+(const C&) は tmp(this)＝共有コピーに += するため
 *     自身の node も書き換わる（原典の挙動。深いコピーではない）
 *
 * 原典からの現代化:
 *   - throw 仕様除去・flatten 非採用（uni.h と同一方針）
 *   - operator+=(const A*) の D::operator+=(_ref)（ポインタを qty__ に
 *     加算＝実体化不能）は原典のまま保存する。呼ぶとコンパイルエラーに
 *     なる（未実装スタブと同じ早期検出の扱い）
 *   - operator=(const A&/const A*) の D 更新は非テンプレート is_qty
 *     （reference_init.h の修正を参照）により文字列リテラルでも安全
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "qty.h"
#include "uni.h"

template <typename A, typename B = A*, typename C = uni__<A, B>, typename D = qty__<qty_>>
class element__ : public C, public D {
 public:
	element__(const A* C_node) : C(C_node), D(this->C::is_qty()){}
	element__(const A* C21_node, const qty_& B_qty) : C(C21_node, B_qty), D(B_qty){}
	element__(const A* C21_node, const qty_* B_qty) : C(C21_node, B_qty), D(B_qty){}
	element__(const C& _C) : C(_C), D(this->C::is_qty()){}
	element__(const C& _C, const qty_& B_qty) : C(_C, B_qty), D(B_qty){}
	element__(const C& _C, const qty_* B_qty) : C(_C, B_qty), D(B_qty){}
	element__(const C* _C, const qty_& B_qty) : C(_C, B_qty), D(B_qty){}
	element__(const C* _C, const qty_* B_qty) : C(_C, B_qty), D(B_qty){}
	element__(const qty_ _qty = 0) : C(_qty), D(_qty){}
	element__(const element__<A, B, C, D>& _element) : C(_element), D(_element){}
	element__(const element__<A, B, C, D>* _element) : C(_element), D(_element){}
	~element__(){}

	/* --- 比較・算術（vs 生値） --- */
	result_ operator> (const A& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const A& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const A& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const A& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const A& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const A& _ref){ return this->C::operator!=(_ref); }
	element__<A, B, C, D> operator+(const A& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	element__<A, B, C, D> operator-(const A& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator-=(_ref); return tmp; }
	element__<A, B, C, D> operator/(const A& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator/=(_ref); return tmp; }
	element__<A, B, C, D> operator*(const A& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator*=(_ref); return tmp; }
	element__<A, B, C, D> operator+(const A* _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); return tmp; }
	element__<A, B, C, D>& operator=(const A& _ref){ this->C::operator=(_ref); this->D::operator=(::is_qty(_ref)); return *this; }
	element__<A, B, C, D>& operator=(const A* _ref){ this->C::operator=(_ref); this->D::operator=(::is_qty(_ref)); return *this; }
	element__<A, B, C, D>& operator+=(const A& _ref){ this->C::operator+=(_ref); return *this; }
	element__<A, B, C, D>& operator-=(const A& _ref){ this->C::operator-=(_ref); return *this; }
	element__<A, B, C, D>& operator/=(const A& _ref){ this->C::operator/=(_ref); return *this; }
	element__<A, B, C, D>& operator*=(const A& _ref){ this->C::operator*=(_ref); return *this; }
	element__<A, B, C, D>& operator+=(const A* _ref){ this->C::operator+=(_ref); this->D::operator+=(_ref); return *this; }

	/* --- 比較・算術（vs element__） --- */
	result_ operator> (const element__<A, B, C, D>& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const element__<A, B, C, D>& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const element__<A, B, C, D>& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const element__<A, B, C, D>& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const element__<A, B, C, D>& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const element__<A, B, C, D>& _ref){ return this->C::operator!=(_ref); }
	element__<A, B, C, D> operator+(const element__<A, B, C, D>& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator+=(_ref); tmp.D::operator+=(_ref); return tmp; }
	element__<A, B, C, D> operator-(const element__<A, B, C, D>& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator-=(_ref); tmp.D::operator-=(_ref); return tmp; }
	element__<A, B, C, D> operator/(const element__<A, B, C, D>& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator/=(_ref); tmp.D::operator/=(_ref); return tmp; }
	element__<A, B, C, D> operator*(const element__<A, B, C, D>& _ref){ element__<A, B, C, D> tmp(this->clone()); tmp.C::operator*=(_ref); tmp.D::operator*=(_ref); return tmp; }
	element__<A, B, C, D> operator+(const element__<A, B, C, D>* _ref);   /* 原典: 宣言のみ（定義なし。使用時リンクエラー） */
	element__<A, B, C, D>& operator=(const element__<A, B, C, D>& _ref){ this->C::operator=(_ref); this->D::operator=(_ref); return *this; }
	element__<A, B, C, D>& operator=(const element__<A, B, C, D>* _ref){ this->C::operator=(_ref); this->D::operator=(_ref); return *this; }
	element__<A, B, C, D>& operator+=(const element__<A, B, C, D>& _ref){ this->C::operator+=(_ref); this->D::operator+=(_ref); return *this; }
	element__<A, B, C, D>& operator-=(const element__<A, B, C, D>& _ref){ this->C::operator-=(_ref); this->D::operator-=(_ref); return *this; }
	element__<A, B, C, D>& operator/=(const element__<A, B, C, D>& _ref){ this->C::operator/=(_ref); this->D::operator/=(_ref); return *this; }
	element__<A, B, C, D>& operator*=(const element__<A, B, C, D>& _ref){ this->C::operator*=(_ref); this->D::operator*=(_ref); return *this; }
	element__<A, B, C, D>& operator+=(const element__<A, B, C, D>* _ref);   /* 原典: 宣言のみ（定義なし。使用時リンクエラー） */

	/* --- 比較・算術（vs C） --- */
	result_ operator> (const C& _ref){ return this->C::operator> (_ref); }
	result_ operator< (const C& _ref){ return this->C::operator< (_ref); }
	result_ operator>=(const C& _ref){ return this->C::operator>=(_ref); }
	result_ operator<=(const C& _ref){ return this->C::operator<=(_ref); }
	result_ operator==(const C& _ref){ return this->C::operator==(_ref); }
	result_ operator!=(const C& _ref){ return this->C::operator!=(_ref); }
	element__<A, B, C, D> operator+(const C& _ref){ element__<A, B, C, D> tmp(this); tmp.C::operator+=(_ref); return tmp; }
	element__<A, B, C, D> operator-(const C& _ref){ element__<A, B, C, D> tmp(this); tmp.C::operator-=(_ref); return tmp; }
	element__<A, B, C, D> operator/(const C& _ref){ element__<A, B, C, D> tmp(this); tmp.C::operator/=(_ref); return tmp; }
	element__<A, B, C, D> operator*(const C& _ref){ element__<A, B, C, D> tmp(this); tmp.C::operator*=(_ref); return tmp; }
	element__<A, B, C, D>& operator=(const C& _ref){ this->C::operator=(_ref); this->D::operator=(_ref.is_qty()); return *this; }
	element__<A, B, C, D>& operator=(const C* _ref){ this->C::operator=(_ref); this->D::operator=(_ref->is_qty()); return *this; }
	element__<A, B, C, D>& operator+=(const C& _ref){ this->C::operator+=(_ref); return *this; }
	element__<A, B, C, D>& operator-=(const C& _ref){ this->C::operator-=(_ref); return *this; }
	element__<A, B, C, D>& operator/=(const C& _ref){ this->C::operator/=(_ref); return *this; }
	element__<A, B, C, D>& operator*=(const C& _ref){ this->C::operator*=(_ref); return *this; }

	/* --- アクセス・複製 --- */
	B& operator()() const { return this->C::operator()(); }
	A& operator()(const idx_ _idx){ return this->C::operator()(_idx); }
	A& operator[](const idx_ _idx){ return this->C::operator()(_idx); }
	element__<A, B, C, D> clone(){
		element__<A, B, C, D> tmp;
		tmp.C::operator=(this->C::clone());
		tmp.D::operator=(this->D::clone());
		return tmp;
	}
	element__<A, B, C, D>* clonep(){
		element__<A, B, C, D>* tmp = new element__<A, B, C, D>;
		tmp->C::operator=(this->C::clone());
		tmp->D::operator=(this->D::clone());
		return tmp;
	}

	/* --- 状態・出力 --- */
	void clear(){ this->C::clear(); this->D::clear(); }
	void info(status_ _status = 0){
		(void)_status;   /* 原典から不使用 */
		this->C::info((status_)0);
		printf("<element__>qty:%ld\n", (long)this->is_qty());
	}
	using D::is_qty;
	using D::check_qty;
	using D::qtying;
	using C::p;
};

/* クラス全体の出力（原典 element.h:226） */
template <typename A, typename B, typename C, typename D>
inline void p(const element__<A, B, C, D>& _ref, status_ _status = 0){
	((element__<A, B, C, D>&)_ref).p();
	newline(_status);
}

#ifndef HAS_ELEM_STRING
typedef element__<char, char*> elem_string_;
typedef element__<char, char*> estring_;
#define HAS_ELEM_STRING
#endif /* HAS_ELEM_STRING */

#endif /* CPP_DEVEL_UNI_ELEMENT_H */

#ifndef CPP_DEVEL_UNI_OPERATOR_BASE_H
#define CPP_DEVEL_UNI_OPERATOR_BASE_H
/*********************************************
 * 統一プリミティブ層: 比較・算術（comp_* / arith_*）
 *
 * 原典: backup/tmp_include/operator_base.h。uni__/qty__ 等の演算子が委譲する。
 *
 * 原典の意味論（保存する）:
 *   - comp_*(A*, A&)  : 配列の「数量」とスカラを比較する（内容ではない）
 *   - comp_*(A*, A*)  : 数量どうしの比較。ただし comp_eq<char> は acmp で内容比較
 *   - arith_*(A*, ...): 要素ごとの演算。配列対配列は数量不一致で bad_range
 *   - arith_add(char*, const char*): 文字列連結。旧 self を
 *     refcnt_decr_or_delete で解放し、生 new[]（refcnt=0）の連結結果を返す。
 *     結果の管理（refcnt_incr）は呼び出し側の責務
 *
 * 原典からの現代化:
 *   - PRE(MNODE_,qty)(p, sizeof(A))（untyped）-> mnode_qty<A>(p)（typed）
 *   - std::bad_range -> ::bad_range、throw 仕様除去
 *   - DP_ARITH / DP_COPY デバッグ計装を除去
 *   - arith_add(char*, const char*) は _self == null のとき原典が
 *     acpy(null, dst) で null 参照していたのを修正（len=0 分岐は
 *     原典にもあったが acpy が無条件だった）
 *********************************************/
#include "../types.h"
#include "../exception.h"
#include "../a/astr.h"
#include "../mem/mnode.h"

/* --- 比較: 配列 vs スカラ（数量比較） --- */
template <typename A> inline result_ comp_gt(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) >  _acc); }
template <typename A> inline result_ comp_lt(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) <  _acc); }
template <typename A> inline result_ comp_ge(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) >= _acc); }
template <typename A> inline result_ comp_le(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) <= _acc); }
template <typename A> inline result_ comp_eq(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) == _acc); }
template <typename A> inline result_ comp_ne(const A* _self, const A& _acc){ return (mnode_qty((A*)_self) != _acc); }

/* --- 比較: スカラ vs スカラ --- */
template <typename A> inline result_ comp_gt(const A& _self, const A& _acc){ return (_self >  _acc); }
template <typename A> inline result_ comp_lt(const A& _self, const A& _acc){ return (_self <  _acc); }
template <typename A> inline result_ comp_ge(const A& _self, const A& _acc){ return (_self >= _acc); }
template <typename A> inline result_ comp_le(const A& _self, const A& _acc){ return (_self <= _acc); }
template <typename A> inline result_ comp_eq(const A& _self, const A& _acc){ return (_self == _acc); }
template <typename A> inline result_ comp_ne(const A& _self, const A& _acc){ return (_self != _acc); }

/* 文字列は内容比較（char のみの特則） */
template <typename A> inline result_ comp_eq(const char* _self, const char* _acc){ return (acmp(_self, _acc) == 0); }

/* --- 比較: 配列 vs 配列（数量比較） --- */
template <typename A> inline result_ comp_gt(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) >  mnode_qty((A*)_acc)); }
template <typename A> inline result_ comp_lt(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) <  mnode_qty((A*)_acc)); }
template <typename A> inline result_ comp_ge(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) >= mnode_qty((A*)_acc)); }
template <typename A> inline result_ comp_le(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) <= mnode_qty((A*)_acc)); }
template <typename A> inline result_ comp_eq(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) == mnode_qty((A*)_acc)); }
template <typename A> inline result_ comp_ne(const A* _self, const A* _acc){ return (mnode_qty((A*)_self) != mnode_qty((A*)_acc)); }

/* --- 算術: スカラ --- */
template <typename A> inline A arith_add(const A& _self, const A& _acc){ return (_self + _acc); }
template <typename A> inline A arith_sub(const A& _self, const A& _acc){ return (_self - _acc); }
template <typename A> inline A arith_mul(const A& _self, const A& _acc){ return (_self * _acc); }
template <typename A> inline A arith_div(const A& _self, const A& _acc){ return (_self / _acc); }
template <typename A> inline A& arith_add_assign(A& _self, const A& _acc){ return (_self = _self + _acc); }
template <typename A> inline A& arith_sub_assign(A& _self, const A& _acc){ return (_self = _self - _acc); }
template <typename A> inline A& arith_mul_assign(A& _self, const A& _acc){ return (_self = _self * _acc); }
template <typename A> inline A& arith_div_assign(A& _self, const A& _acc){ return (_self = _self / _acc); }

/* --- 算術: 配列 対 配列（数量一致が前提。結果は生 new[]・refcnt=0） --- */
template <typename A> inline A* arith_add(const A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] + _acc[i]; }
	return tmp;
}
template <typename A> inline A* arith_sub(const A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] - _acc[i]; }
	return tmp;
}
template <typename A> inline A* arith_mul(const A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] * _acc[i]; }
	return tmp;
}
template <typename A> inline A* arith_div(const A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] / _acc[i]; }
	return tmp;
}
template <typename A> inline A* arith_add_assign(A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] += _acc[i]; }
	return _self;
}
template <typename A> inline A* arith_sub_assign(A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] -= _acc[i]; }
	return _self;
}
template <typename A> inline A* arith_mul_assign(A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] *= _acc[i]; }
	return _self;
}
template <typename A> inline A* arith_div_assign(A* _self, const A* _acc){
	qty_ tmpqty = mnode_qty(_self); qty_ accqty = mnode_qty(_acc);
	if ( tmpqty != accqty ) throw bad_range();
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] /= _acc[i]; }
	return _self;
}

/* --- 算術: 配列 対 スカラ --- */
template <typename A> inline A* arith_add(const A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] + _acc; }
	return tmp;
}
template <typename A> inline A* arith_sub(const A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] - _acc; }
	return tmp;
}
template <typename A> inline A* arith_mul(const A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] * _acc; }
	return tmp;
}
template <typename A> inline A* arith_div(const A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	A* tmp = new A[tmpqty];
	for ( qty_ i = 0; i < tmpqty; ++i ){ tmp[i] = _self[i] / _acc; }
	return tmp;
}
template <typename A> inline A* arith_add_assign(A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] += _acc; }
	return _self;
}
template <typename A> inline A* arith_sub_assign(A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] -= _acc; }
	return _self;
}
template <typename A> inline A* arith_mul_assign(A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] *= _acc; }
	return _self;
}
template <typename A> inline A* arith_div_assign(A* _self, const A& _acc){
	qty_ tmpqty = mnode_qty(_self);
	for ( qty_ i = 0; i < tmpqty; ++i ){ _self[i] /= _acc; }
	return _self;
}

/* --- 文字列連結（char の特則）。旧 self を解放し、生 new[]（refcnt=0）を返す --- */
template <typename A> inline A* arith_add(char* _self, const char* _acc){
	unsigned long tmplen = alen(_acc);
	unsigned long len;
	if ( _self ) len = alen(_self); else len = 0;
	char* tmp = new char[tmplen + len + 1];
	if ( _self ) acpy(_self, tmp);            /* 原典は null でも acpy していた（修正） */
	mnode_refcnt_decr_or_delete(_self);
	acpy((char*)_acc, tmp + len);
	return tmp;
}

#endif /* CPP_DEVEL_UNI_OPERATOR_BASE_H */

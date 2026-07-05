#ifndef CPP_DEVEL_UNI_REFERENCE_INIT_H
#define CPP_DEVEL_UNI_REFERENCE_INIT_H
/*********************************************
 * 統一プリミティブ層: 確保・初期化・共有・複製・解放（init/copy/clone/uninit）
 *
 * 原典: backup/tmp_include/reference_init.h（codegen 出力スナップショット）。
 * uni__ の全 ctor/dtor/operator= がここへ委譲する。所有権プロトコル:
 *
 *   init_qty<A>(qty)      : new A[qty] + ゼロ充填 + refcnt_incr -> refcnt=1
 *   init(共有)            : refcnt_incr で共有参照を追加
 *   copy(代入)            : 旧 node を refcnt_decr_or_delete してから init
 *   uninit(破棄)          : refcnt_decr_or_delete（0 到達で delete[]）
 *
 * char（文字列）の特則:
 *   - 確保は常に qty+1（NUL スロット）
 *   - 文字列リテラル等アリーナ外のソースは深いコピー、アリーナ内は共有
 *
 * 原典からの現代化:
 *   - throw(std::bad_ctor) 動的例外仕様を除去（C++17）。std::bad_ctor ->
 *     ::bad_ctor（exception.h。std 注入の廃止）
 *   - PRE(MNODE_,x) トークン連結 -> mnode_x 実名
 *   - mnode_qty(_acc, sizeof(A))（untyped）-> mnode_qty<A>(_acc)（typed。
 *     cookie 付き配列でも正しい。トリビアル型では原典と同値）
 *   - __attribute__((flatten)) は非採用（最適化ヒント。意味論に影響しない）
 *   - 原典の init(A**, const A*, qty, idx) は本体が printf("not implimented")
 *     の未実装スタブだった。移植せず、呼ぶとコンパイルエラーになるようにする
 *     （サイレントな実行時素通りよりも早期検出を優先）
 *********************************************/
#include "../types.h"
#include "../exception.h"
#include "../a/astr.h"
#include "../mem/mnode.h"
#include "../mem/mallocator.h"

/* --- 確保（qty 指定） --- */

template <typename A>
inline A* init_qty(const qty_ _qty){
	A* _self = new A[_qty];
	for ( long i = 0; i < _qty; ++i ){ _self[i] = 0; }
	mnode_refcnt_incr(_self);
	return _self;
}

/* char: NUL スロットぶん +1 で確保。qty <= 0 は空（null） */
template <typename A>
inline qty_ init_qty(char** _self, qty_ _qty){
	if ( _qty <= 0 ){ *_self = 0; }
	else            { *_self = init_qty<A>(_qty + 1); }
	return _qty;
}

/* 一般配列: qty==0 は既定容量 0x20、負は bad_ctor */
template <typename A>
inline qty_ init_qty(A** _self, qty_ _qty){
	if ( _qty == 0 ) _qty = 0x20;
	if ( _qty <= 0 ) throw bad_ctor();
	*_self = init_qty<A>(_qty);
	return _qty;
}

/* スカラ: 「qty で初期化」は値の代入（統一インタフェースの規約） */
template <typename A>
inline qty_ init_qty(A* _self, qty_ _qty){
	*_self = _qty;
	return _qty;
}

/* --- 初期化・共有 --- */

template <typename A>
inline void init(A** _self, const A& _acc){
	::init_qty<A>(_self, 0x20);
	(*_self)[0] = _acc;
}

template <typename A>
inline void init(A* _self, const A& _acc){
	*_self = _acc;
}

/* 共有: 参照を増やして同じ node を指す */
template <typename A>
inline void init(A** _self, const A* _acc){
	mnode_refcnt_incr((A*)_acc);
	*_self = (A*)_acc;
}

template <typename A>
inline void init(char** _self, const char& _acc){
	::init_qty<A>(_self, 2);
	(*_self)[0] = _acc;
}

/* 文字列: アリーナ内は共有、外（リテラル・スタック）は深いコピー */
template <typename A>
inline void init(char** _self, const char* _acc){
	if ( mallocator.is_memory(_acc) ){
		mnode_refcnt_incr((A*)_acc);
		*_self = (A*)_acc;
	} else {
		if ( _acc != 0 ){
			qty_ qty = alen(_acc);
			::init_qty<A>(_self, qty);
			ancpy(_acc, *_self, qty);
		}
	}
}

template <typename A>
inline void init(A* _self, const A* _acc){
	*_self = _acc[0];
}

template <typename A>
inline void init(A* _self, const A* _acc, qty_ _qty){
	*_self = _acc[_qty];
}

/* qty 指定つき: アリーナ内で同じ qty なら共有、それ以外は確保して複写 */
template <typename A>
inline void init(A** _self, const A* _acc, qty_ _qty){
	qty_ tmpqty = 0;
	if ( mallocator.is_memory(_acc) ){
		if ( (tmpqty = mnode_qty(_acc)) == _qty ){ init<A>(_self, _acc); return; }
	}
	::init_qty<A>(_self, _qty);
	if ( tmpqty != 0 && tmpqty < _qty ) _qty = tmpqty;
	for ( long i = 0; i < _qty; ++i ) (*_self)[i] = _acc[i];
}

template <typename A>
inline void init(char** _self, const char* _acc, qty_ _qty){
	qty_ tmplen = alen(_acc);
	if ( mallocator.is_memory(_acc) ){
		if ( tmplen == _qty ){ init<A>(_self, _acc); return; }
	}
	::init_qty<A>(_self, _qty);
	if ( tmplen < _qty ) _qty = tmplen;
	for ( long i = 0; i < _qty; ++i ) (*_self)[i] = _acc[i];
}

template <typename A>
inline void init(A** _self, const A& _acc, qty_ _qty, idx_ _idx){
	::init_qty<A>(_self, _qty);
	(*_self)[_idx] = _acc;
}

/* 原典の init(A**, const A*, qty_, idx_) は未実装スタブのため移植しない */

/* --- 代入（旧参照を解放してから初期化） --- */

template <typename A>
inline void copy(A* _self, const A& _acc){
	*_self = _acc;
}

template <typename A>
inline void copy(A** _self, const A* _acc){
	/* 自己代入ガード（修正）: 原典は refcnt==1 の自己代入で
	   decr_or_delete が node を解放した後に共有し use-after-free になる。
	   同一 node なら参照数の純変化も 0 なので何もしないのが正しい。 */
	if ( *_self == _acc ) return;
	mnode_refcnt_decr_or_delete(*_self);
	::init<A>(_self, _acc);
}

template <typename A>
inline void copy(A** _self, const A& _acc){
	(*_self)[0] = _acc;
}

template <typename A>
inline void copy(A* _self, const A* _acc){
	*_self = _acc[0];
}

/* --- 複製（深いコピー） --- */

template <typename A>
inline A* clone(A** _self, const A* _acc){
	mnode_refcnt_decr_or_delete(*_self);
	if ( !mallocator.is_memory(_acc) ){ return 0; }
	qty_ qty = mnode_qty(_acc);
	*_self = init_qty<A>(qty);
	for ( long i = 0; i < qty; ++i ){ (*_self)[i] = _acc[i]; }
	return *_self;
}

template <typename A>
inline A* clone(A* _self, const A& _acc){
	*_self = _acc;
	return _self;
}

/* --- 解放 --- */

template <typename A>
inline void uninit(A&){}

template <typename A>
inline void uninit(A* _ref){
	mnode_refcnt_decr_or_delete(_ref);
}

/* --- 数量 --- */

template <typename A>
inline qty_ is_qty(const char* _ref){
	if ( !_ref ) return 0;
	return alen(_ref);
}

/* 非テンプレート版（修正）: element__ 等の「テンプレート引数なし」の
   ::is_qty(_ref) 呼び出しは、char* 版テンプレートの A が推定不能のため
   総称の mnode_qty 版に落ち、文字列リテラル（アリーナ外）に対して
   mnode ヘッダ読みの未定義動作になる。非テンプレートを最優先で解決
   させることで alen 経路に固定する。明示 <A> 呼び出し（uni__ 内部）の
   解決には影響しない。 */
inline qty_ is_qty(const char* _ref){
	if ( !_ref ) return 0;
	return alen(_ref);
}

template <typename A>
inline qty_ is_qty(const A* _ref){
	if ( !_ref ) return 0;
	return mnode_qty(_ref);
}

template <typename A>
inline qty_ is_qty(const A){
	return 1;
}

#endif /* CPP_DEVEL_UNI_REFERENCE_INIT_H */

#ifndef CPP_DEVEL_UNI_LIST_H
#define CPP_DEVEL_UNI_LIST_H
/*********************************************
 * list__<A,B,C> — 双方向循環リスト（原典 backup/tmp_include/list.h）
 *
 *   list__ : public ballocator__<lnode__<A,B,C>>
 *   ノードは balloc したスラブから供給する（個別 new しない）。
 *   sentinel の elem を含む循環リストで、elem.next..last の手前までが
 *   使用中、last..underlast が空き鎖。add は *last への代入 + countup
 *   （last を進める）、del はノードを clear して last の直前へ返す。
 *   空き鎖が尽きる（last == &elem）と enlarge が新スラブを継ぎ足す。
 *
 *   既定テンプレート引数は B=A（uni__ の B=A* と異なる）。文字列リストは
 *   明示指定の slist_ = list__<char, char*, string_> を使う。
 *
 * 検索・比較の意味論（uni_ の数量比較に従う。原典どおり）:
 *   - search(C 系) は operator==（char は内容比較、他の配列は数量比較）
 *   - simple_sort は operator>（char 配列＝文字列は「長さ」で昇順）
 *
 * 原典からの現代化・修正（bug: が原典バグの修正）:
 *   - bug: search(C&) / search_t_ndx(C&) の simple_searc 系タイポ
 *     （未実体化で顕在化しなかった）
 *   - bug: insert() の enlarge 条件 qty == is_qty() は恒真（毎回スラブを
 *     倍化確保）。add と同じ「空き鎖が尽きたら」に修正。また原典は
 *     ノード移設後に countup していて last が移設先の隣（使用中ノード）を
 *     指し空き鎖が壊れる。先に countup で last を進めてから移設する
 *   - bug: insert() の tmp->elem=data（lnode に elem メンバはない）->
 *     *tmp = data
 *   - bug: move_before/move_after の this->node（list に node メンバは
 *     ない）-> this->elem。to->next（protected）-> to->is_next()
 *   - simple_sort の self->elem 比較（同上）-> ノードどうしの operator>。
 *     隣接判定より先に last 到達を検査する（原典は last の値も比較して
 *     いた。挙動同値・無駄比較の除去のみ）
 *   - insert() の SAFE_LVL ガード（_ref==last 拒否）は常時有効化
 *   - DP デバッグ計装の除去、PLD 書式マクロは %08 書式に展開
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "../result.h"
#include "../cond.h"
#include "../exception.h"
#include "string.h"
#include "lnode.h"
#include "ballocator.h"

template <typename A, typename B = A, typename C = uni__<A, B>>
class list__ : public ballocator__<lnode__<A, B, C>> {
 protected:
	lnode__<A, B, C> elem;                 /* sentinel（循環の継ぎ目） */
	qty_ qty;                              /* 使用中ノード数 */
	lnode__<A, B, C>* last;                /* 先頭の空きノード */
	lnode__<A, B, C>* underlast;           /* 最後尾の空きノード */
 public:
	list__(qty_ _qty = 0x20) : ballocator__<lnode__<A, B, C>>(0x20), elem((qty_)0), qty(0){
		lnode__<A, B, C>* tmp = this->balloc(_qty);
		if ( is_check(tmp) ){
			for ( long i = 0; i < _qty - 1; i++ ){ tmp->connect(&tmp[i], &tmp[i + 1]); }
			elem.connect_r(&elem, &elem, &tmp[0], &tmp[_qty - 1]);
			this->underlast = &tmp[_qty - 1];
			last = elem.is_next();
		} else { throw bad_ctor(); }
	}
	virtual ~list__(){
		lnode__<A, B, C>* tmp = elem.is_next();
		while ( tmp != last ){
			tmp->clear();
			tmp = tmp->is_next();
		}
	}

	result_ enlarge(long val = 1){
		long grow_size = this->ballocator__<lnode__<A, B, C>>::is_qty()
		               + this->ballocator__<lnode__<A, B, C>>::is_qty() * val;
		lnode__<A, B, C>* tmp = this->balloc(grow_size);
		if ( !is_check(tmp) ){ return ERR_C(LIST_BALLOC_FAIL); }
		for ( long i = 0; i < grow_size - 1; i++ ){ tmp->connect(&tmp[i], &tmp[i + 1]); }
		elem.connect_r(&elem, this->underlast, &tmp[0], &tmp[grow_size - 1]);
		this->last = &tmp[0];
		this->underlast = &tmp[grow_size - 1];
		return TRUE;
	}

	lnode__<A, B, C>* head(){ return elem.is_next(); }
	lnode__<A, B, C>* tail(){ return last->is_prev(); }
	inline lnode__<A, B, C>* next_elem(lnode__<A, B, C>* ref){ return ref->is_next(); }
	inline lnode__<A, B, C>* prev_elem(lnode__<A, B, C>* ref){ return ref->is_prev(); }
	void countup(){ ++qty; last = last->is_next(); }
	void countdown(){ --qty; last = last->is_prev(); }
	qty_ is_qty(){ return this->qty; }
	qty_ is_base_qty(){ return this->ballocator__<lnode__<A, B, C>>::is_qty(); }

	/* --- 追加（*last へ代入して countup） --- */
	C* add(B _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = _data; C* tmp = last; this->countup(); return tmp;
	}
	C* add(B& _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }   /* 原典は戻り値なしで素通り（修正） */
		*last = _data; C* tmp = last; this->countup(); return tmp;
	}
	C* add(B* _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = _data; C* tmp = last; this->countup(); return tmp;
	}
	C* add(C _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = (C)_ref; C* tmp = last; this->countup(); return tmp;
	}
	C* add(C* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = *_ref; C* tmp = last; this->countup(); return tmp;
	}
	C* add(lnode__<A, B, C>& _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = _ref; C* tmp = last; this->countup(); return tmp;
	}
	C* add(lnode__<A, B, C>* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)NUL_(LIST_LISTIN); } }
		*last = *_ref; C* tmp = last; this->countup(); return tmp;
	}

	/* --- 追加して序数を返す --- */
	ndx_ add_t_ndx(B _data){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		*last = _data; this->countup(); ndx_ tmp = search_t_ndx(_data); return tmp;
	}
	ndx_ add_t_ndx(B& _data){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		*last = _data; this->countup(); ndx_ tmp = search_t_ndx(_data); return tmp;
	}
	ndx_ add_t_ndx(B* _data){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		*last = _data; this->countup(); ndx_ tmp = search_t_ndx(_data); return tmp;
	}
	ndx_ add_t_ndx(C _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		*last = (C)_ref; this->countup(); ndx_ tmp = search_t_ndx(_ref); return tmp;
	}
	ndx_ add_t_ndx(C* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		*last = *_ref; this->countup(); ndx_ tmp = search_t_ndx(_ref); return tmp;
	}
	ndx_ add_t_ndx(lnode__<A, B, C>& _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		(*last)() = _ref(); this->countup(); ndx_ tmp = search_t_ndx(_ref); return tmp;
	}
	ndx_ add_t_ndx(lnode__<A, B, C>* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return ERR_N(LIST_LISTIN_T_NDX_ENLARGE_FAIL); } }
		(*last)() = (*_ref)(); this->countup(); ndx_ tmp = search_t_ndx(_ref); return tmp;
	}

	/* --- 追加して複製を返す --- */
	C add_t_clone(B _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(B& _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(B* _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(C _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = (C)_ref; C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(C* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = *_ref; C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(lnode__<A, B, C>& _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		(*last)() = _ref(); C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C add_t_clone(lnode__<A, B, C>* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		(*last)() = (*_ref)(); C* tmp = last; this->countup(); return tmp->C::clone();
	}
	C* add_t_clonep(B _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(B& _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(B* _data){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = _data; C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(C _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = (C)_ref; C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(C* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		*last = *_ref; C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(lnode__<A, B, C>& _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		(*last)() = _ref(); C* tmp = last; this->countup(); return tmp->C::clonep();
	}
	C* add_t_clonep(lnode__<A, B, C>* _ref){
		if ( last == &elem ){ if ( !enlarge() ){ return (C*)0; } }
		(*last)() = (*_ref)(); C* tmp = last; this->countup(); return tmp->C::clonep();
	}

	/* --- 削除（ノードを clear して空き鎖へ返す） --- */
	result_ del_byidx(idx_ _idx){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; _idx > 0; --_idx, tmp = tmp->is_next() ){
			if ( tmp == last ){ return ERR_N(LIST_DEL_BYIDX_SEARCH_FAIL); }
		}
		this->del(tmp);
		return TRUE;
	}
	result_ del(B _ref){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; tmp != last; tmp = tmp->is_next() ){
			if ( (*tmp)() == _ref ){ return this->del(tmp); }
		}
		return ERR_N(LIST_DEL_BYTYPE_SEARCH_FAIL);
	}
	result_ del(C _ref){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; tmp != last; tmp = tmp->is_next() ){
			if ( (*tmp) == _ref ){ return this->del(tmp); }
		}
		return ERR_N(LIST_DEL_BYTYPE_SEARCH_FAIL);
	}
	result_ del(C& _ref){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; tmp != last; tmp = tmp->is_next() ){
			if ( (*tmp) == _ref ){ return this->del(tmp); }
		}
		return ERR_N(LIST_DEL_BYTYPE_SEARCH_FAIL);
	}
	result_ del(C* _ref){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; tmp != last; tmp = tmp->is_next() ){
			if ( (*tmp) == *_ref ){ return this->del(tmp); }
		}
		return ERR_N(LIST_DEL_BYTYPE_SEARCH_FAIL);
	}
	result_ del(lnode__<A, B, C>* _ref){
		_ref->clear();
		this->elem.connect_f(_ref);
		this->elem.connect_i(last, _ref);
		this->countdown();
		return TRUE;
	}

	/* --- 挿入・移動 --- */
	result_ insert(lnode__<A, B, C>* _ref, B _data){
		if ( _ref == last ){ return FALSE; }                        /* 原典 SAFE_LVL ガード（常時有効化） */
		if ( last == &elem ){ if ( !enlarge() ){ return FALSE; } }  /* 修正: 原典は恒真条件で毎回 enlarge */
		lnode__<A, B, C>* tmp = last;
		this->countup();                                            /* 修正: 移設前に last を進める */
		this->elem.connect_f(tmp);
		this->elem.connect_i(_ref, tmp);
		*tmp = _data;                                               /* 修正: 原典 tmp->elem=data */
		return TRUE;
	}
	result_ move_before(lnode__<A, B, C>* ref, lnode__<A, B, C>* to){
		this->elem.connect_f(ref);                                  /* 修正: 原典 this->node */
		this->elem.connect_i(to, ref);
		return TRUE;
	}
	result_ move_after(lnode__<A, B, C>* ref, lnode__<A, B, C>* to){
		this->elem.connect_f(ref);                                  /* 修正: 原典 this->node */
		this->elem.connect_i(to->is_next(), ref);                   /* 修正: to->next は protected */
		return TRUE;
	}

	/* --- 検索 --- */
	C* search(B _ref, cnt_ _cnt = 0){ return simple_search(_ref, _cnt); }
	C* search(C _ref, cnt_ _cnt = 0){ return simple_search(&_ref, _cnt); }
	C* search(C& _ref, cnt_ _cnt = 0){ return simple_search(&_ref, _cnt); }   /* 修正: 原典 simple_searc */
	C* search(C* _ref, cnt_ _cnt = 0){ return simple_search(_ref, _cnt); }
	C* simple_search(B _ref, cnt_ _skip = 0){
		lnode__<A, B, C>* tmp = elem.is_next();
		while ( tmp != last ){
			if ( tmp->C::operator==(_ref) ){ if ( _skip <= 0 ) return (C*)tmp; else --_skip; }
			tmp = tmp->is_next();
		}
		return (C*)NUL_(LIST_SEARCH_FAIL);
	}
	C* simple_search(C _ref, cnt_ _skip = 0){ return this->simple_search(&_ref, _skip); }
	C* simple_search(C& _ref, cnt_ _skip = 0){ return this->simple_search(&_ref, _skip); }
	C* simple_search(C* _ref, cnt_ _skip = 0){
		lnode__<A, B, C>* tmp = elem.is_next();
		while ( tmp != last ){
			if ( (*tmp) == *_ref ){ if ( _skip <= 0 ) return (C*)tmp; else --_skip; }
			tmp = tmp->is_next();
		}
		return (C*)NUL_(LIST_SEARCH_FAIL);
	}
	ndx_ search_t_ndx(B _ref, cnt_ _cnt = 0){ return simple_search_t_ndx(_ref, _cnt); }
	ndx_ search_t_ndx(C _ref, cnt_ _cnt = 0){ return simple_search_t_ndx(&_ref, _cnt); }
	ndx_ search_t_ndx(C& _ref, cnt_ _cnt = 0){ return simple_search_t_ndx(&_ref, _cnt); }   /* 修正: 原典 simple_searc_t_ndx */
	ndx_ search_t_ndx(C* _ref, cnt_ _cnt = 0){ return simple_search_t_ndx(_ref, _cnt); }
	ndx_ simple_search_t_ndx(B _ref, cnt_ _skip = 0){
		ndx_ ndx = 0;
		lnode__<A, B, C>* tmp = elem.is_next();
		while ( tmp != last ){
			if ( tmp->C::operator==(_ref) ){ if ( _skip <= 0 ){ return ndx; } else --_skip; }
			tmp = tmp->is_next();
			++ndx;
		}
		return ERR_N(LIST_SEARCH_IDX_FAIL);
	}
	idx_ simple_search_t_ndx(C* _ref, cnt_ _skip = 0){
		ndx_ ndx = 0;
		lnode__<A, B, C>* tmp = elem.is_next();
		while ( tmp != last ){
			if ( (*tmp) == *_ref ){ if ( _skip <= 0 ){ return ndx; } else --_skip; }
			tmp = tmp->is_next();
			++ndx;
		}
		return ERR_N(LIST_SEARCH_IDX_FAIL);
	}

	/* --- 整列（operator> 昇順。文字列は uni_ の数量＝長さ比較） --- */
	result_ simple_sort(){
		lnode__<A, B, C>* self = elem.is_next();
		lnode__<A, B, C>* acc = self->is_next();
		while ( self->is_next() != last ){
			while ( acc != last ){
				if ( (*self) > (*acc) ){
					while ( acc->is_next() != last && (*self) > (*(acc->is_next())) ){ acc = acc->is_next(); }
					elem.connect_b(self, acc);
					self = acc;
					acc = self->is_next();
				} else { acc = acc->is_next(); }
			}
			self = self->is_next();
			acc = self->is_next();
		}
		return TRUE;
	}

	/* --- 添字アクセス（使用中ノードの序数。境界検査なし＝原典どおり） --- */
	lnode__<A, B, C>& operator[](idx_ _idx){
		lnode__<A, B, C>* tmp = this->elem.is_next();
		for ( ; _idx > 0; --_idx ){ tmp = tmp->is_next(); }
		return (lnode__<A, B, C>&)(*tmp);
	}
	lnode__<A, B, C>& operator()(){
		lnode__<A, B, C>* tmp = this->elem.is_next();
		return (lnode__<A, B, C>&)(*tmp);
	}
	lnode__<A, B, C>& operator()(idx_ _idx){
		lnode__<A, B, C>* tmp = this->elem.is_next();
		for ( ; _idx > 0; --_idx ){ tmp = tmp->is_next(); }
		return (lnode__<A, B, C>&)(*tmp);
	}

	/* --- 出力 --- */
	void info(){
		printf("<sizeof(elem):%08zu> <qty:%08ld> <last:%p> <underlast:%p> <&elem:%p>\n",
		       sizeof(elem), qty, (void*)last, (void*)underlast, (void*)&elem);
	}
	void node_info(idx_ _idx = 0){
		lnode__<A, B, C>* tmp = elem.is_next();
		for ( ; _idx > 0; --_idx, tmp = tmp->is_next() ){}
		tmp->info();
	}
	void show(){
		lnode__<A, B, C>* tmp = elem.is_next();
		long i = 0;
		printf("--show list"); newline();
		for ( ; i < qty; i++ ){ tmp->info(); tmp = tmp->is_next(); }
		printf("node=%ld--\n", i);
	}
	void show_base(qty_ _qty = 0x10){
		lnode__<A, B, C>* tmp = elem.is_next();
		printf("*SENTINEL*\n");
		this->elem.info();
		for ( long i = 0; i < _qty && tmp != underlast; ++i, tmp = tmp->is_next() ){ tmp->info(); }
		tmp->info();
	}

	/* --- 複製 --- */
	list__<A, B, C> clone(){
		list__<A, B, C> tmp;
		tmp.ballocator__<lnode__<A, B, C>>::operator=(this->ballocator__<lnode__<A, B, C>>::clone());
		return tmp;
	}
	list__<A, B, C>* clonep(){
		list__<A, B, C>* tmp = new list__<A, B, C>;
		tmp->ballocator__<lnode__<A, B, C>>::operator=(this->ballocator__<lnode__<A, B, C>>::clone());
		return tmp;
	}
};

typedef list__<char, char*, string_> slist_;

#endif /* CPP_DEVEL_UNI_LIST_H */

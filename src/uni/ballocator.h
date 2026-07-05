#ifndef CPP_DEVEL_UNI_BALLOCATOR_H
#define CPP_DEVEL_UNI_BALLOCATOR_H
/*********************************************
 * ballocator__<A,B> — スラブ束アロケータ（原典 backup/tmp_include/ballocator.h）
 *
 *   ballocator__ : public B,  B = allocator_element__<A*, A**>（既定）
 *   allocator_node（A* のスロット表）に balloc() が new A[qty] のスラブを
 *   登録していく。list__ の基底としてノード用スラブを供給する。
 *
 *   - balloc(qty)  : 空きスロットに new A[qty] を置き refcnt=1、qty 累計を加算
 *   - node_avail() : 先頭の空きスロット添字。満杯は ERR_N(BALLOC_LIST_OVER)
 *                    （負値。is_check_idx で検査＝result.h の符号拡張が前提）
 *   - ~ballocator__: 全スロットを refcnt_decr_or_delete で解放
 *   - is_qty()     : スラブ内ノードの累計（B 側 allocator_node の
 *                    is_qty を隠蔽する。スロット数は is_base_qty()）
 *
 * 原典からの現代化:
 *   - PRE(MNODE_,x) トークン連結の実名化、PRE(is_,qty) -> is_qty
 *   - D_MNODE / DP デバッグ計装の除去
 *   - DEBUG 専用の show/info は printf 書式を LP64 に合わせて保存。
 *     is_balloc_node は原典どおり宣言のみ（定義がスナップショットにない）
 *   - virtual dtor は原典どおり保存（list__ が継承・多相破棄に備える）
 *********************************************/
#include "../types.h"
#include "../result.h"
#include "../cond.h"
#include "../mem/mnode.h"
#include "allocator_element.h"   /* DEBUG 時の printf/newline もここ経由で可視 */

template <typename A, typename B = allocator_element__<A*, A**>>
class ballocator__ : public B {
 protected:
	qty_ qty;
	idx_ node_avail(){
		for ( idx_ i = 0; i < this->is_base_qty(); ++i ){
			if ( this->allocator_node[i] == 0 ) return i;
		}
		return ERR_N(BALLOC_LIST_OVER);
	}
 public:
	ballocator__(qty_ _qty = 0x20) : B(_qty){ this->qty = 0; }
	virtual ~ballocator__(){
		idx_ max_cnt = this->is_base_qty();
		for ( idx_ cnt = 0; cnt < max_cnt; ++cnt ){
			if ( this->allocator_node[cnt] != 0 ){
				mnode_refcnt_decr_or_delete(this->allocator_node[cnt]);
			}
		}
	}

	A* balloc(qty_ _qty){
		idx_ _idx = this->node_avail();
		if ( is_check_idx(_idx) ){
			this->allocator_node[_idx] = (A*)new A[_qty];
			mnode_refcnt_incr(this->allocator_node[_idx]);
			this->qty = this->qty + _qty;
			return this->allocator_node[_idx];
		}
		return (A*)NUL_(ERR_ballocator__BALLOC);
	}

	qty_ is_qty(){ return this->qty; }
	/* is_full(): 引数なし版は「空きスロットがあるか」を返す（原典の名前と
	   逆に読める挙動だが原典どおり保存） */
	bool is_full(){ idx_ idx = node_avail(); return is_check_idx(idx); }
	bool is_full(idx_ _idx){ return (_idx >= 0 && _idx < this->qty); }

#ifdef DEBUG
	void show(){
		printf("********************************"); newline();
		for ( size_ i = 0; i < (size_)this->is_base_qty(); ++i ){
			printf("%7lu : %p : %p\n", i, (void*)&this->allocator_node[i], (void*)this->allocator_node[i]);
		}
		printf("********************************"); newline();
	}
	void info(idx_ _idx = 0){
		printf("<qty:%08ld> <base_qty:%08ld> ", this->is_qty(), this->is_base_qty());
		printf("<node top addr:%p> <value:%p>\n", (void*)&this->allocator_node[_idx], (void*)this->allocator_node[_idx]);
	}
	void is_balloc_node(idx_ _idx = 0);   /* 原典: 宣言のみ（定義なし） */
#endif /* DEBUG */

	ballocator__<A, B> clone(){
		ballocator__<A, B> tmp;
		tmp.B::operator=(this->B::clone());
		return tmp;
	}
	ballocator__<A, B>* clonep(){
		ballocator__<A, B>* tmp = new ballocator__<A, B>;
		tmp->B::operator=(this->B::clone());
		return tmp;
	}
};

#endif /* CPP_DEVEL_UNI_BALLOCATOR_H */

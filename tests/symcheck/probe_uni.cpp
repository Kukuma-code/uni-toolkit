/*********************************************
 * symcheck 被験体（本番コード uni__ 版）
 *
 * 汎用フィクスチャで示す性質を、実際の再構築コード uni__<long,long*> でも
 * 証明する。配列実体化の「使う想定」だけを odr-use する:
 *   使う:   ctor(qty_), operator[], is_qty, dtor, （経由で check_qty / bad_range）
 *   使わない: clone, clonep, operator/, operator*(uni,uni), 比較演算子, copy ctor
 *
 * 目的4（境界チェック常駐）の被験体でもある: operator[] を使うと
 * check_qty と bad_range が必ずオブジェクトに現れる（境界検査が
 * コンパイルされ落とされていない）ことを symcheck が確認する。
 *********************************************/
#include "uni/uni.h"

long probe_uni_index(){
	uni__<long, long*> a((qty_)4);   /* ctor(qty_) -> init_qty (new[] + refcnt) */
	a[2] = 7;                        /* operator[] -> operator()(A*,idx) -> check_qty / bad_range */
	return a[2] + a.is_qty();        /* is_qty */
}                                    /* dtor -> uninit -> refcnt_decr_or_delete */

long probe_uni_anchor(){ return probe_uni_index(); }

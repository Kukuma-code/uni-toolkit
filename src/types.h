#ifndef CPP_DEVEL_TYPES_H
#define CPP_DEVEL_TYPES_H
/*********************************************
 * 原典 devel/include/types.h の scalar typedef 群のうち、
 * 現時点で使用するものだけを持ち込む（未使用型は持ち込まない方針）。
 * host = 64bit（原典の LONG64 系統に相当）。
 * 値の意味は原典どおり: r* = signed（既定）, l* = unsigned。
 *********************************************/

typedef unsigned long size_;    /* 確保サイズ・バイト数 */

typedef unsigned long lqty_;
typedef long          rqty_;
typedef rqty_         qty_;     /* 要素数 */

typedef unsigned long lcnt_;
typedef long          rcnt_;
typedef rcnt_         cnt_;     /* カウンタ（refcnt 等） */

typedef unsigned long lidx_;
typedef long          ridx_;
typedef ridx_         idx_;     /* 添字 */

typedef unsigned long lndx_;
typedef long          rndx_;
typedef rndx_         ndx_;     /* 探索結果の序数（list の *_t_ndx 系） */

typedef long          result_;  /* 比較・成否の結果 */
typedef unsigned long status_;  /* 出力フォーマット等の状態ビット */

#endif /* CPP_DEVEL_TYPES_H */

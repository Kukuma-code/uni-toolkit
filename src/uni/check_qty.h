#ifndef CPP_DEVEL_UNI_CHECK_QTY_H
#define CPP_DEVEL_UNI_CHECK_QTY_H
/*********************************************
 * 添字境界チェック（原典 include/pre.h の自由関数 check_qty）。
 * uni__::check_qty -> ::check_qty(_idx, is_qty(), node) が呼ぶ。
 * 第3引数は node の型でオーバーロードを選ぶためだけに使う（値は見ない）。
 *
 * 原典は総称で `0 <= _ref && _ref <= _val`（idx == qty を許容）だった。
 *   - char 配列: [qty] は NUL スロットで、init_qty の +1 確保に支えられ正当。
 *     NUL 終端を読む文字列走査が依存するため許容を維持する。
 *   - 非 char 配列: [qty] は new A[qty] の外＝真の OOB（原典のバグ）。
 *     ポインタ型別オーバーロードで厳密化する（修正点）。
 *   - スカラ node: uni__ のスカラ operator() は idx を使わないため原典どおり。
 * 原典の <char> 特殊化（スカラ char で _val+1 許容）はスカラ経路が idx を
 * 参照しないため移植しない。
 *********************************************/

/* スカラ node（原典どおり idx == qty 許容。参照されない） */
template <typename A>
inline bool check_qty(long _ref, long _val, A){ return 0 <= _ref && _ref <= _val; }

/* 非 char 配列: 厳密境界（原典の [qty] OOB を修正） */
template <typename A>
inline bool check_qty(long _ref, long _val, A*){ return 0 <= _ref && _ref < _val; }

/* char 配列: NUL スロット [qty] を許容（+1 確保が保証） */
inline bool check_qty(long _ref, long _val, char*){ return 0 <= _ref && _ref <= _val; }
inline bool check_qty(long _ref, long _val, const char*){ return 0 <= _ref && _ref <= _val; }

#endif /* CPP_DEVEL_UNI_CHECK_QTY_H */

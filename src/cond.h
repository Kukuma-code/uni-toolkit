#ifndef CPP_DEVEL_COND_H
#define CPP_DEVEL_COND_H
/*********************************************
 * 検査プリミティブ（原典 include/cond.h の __cplusplus ブロック）。
 *
 * ballocator__/list__ がエラー戻り値の判定に使う is_check / is_check_idx
 * のみ移植する。原典 cond.h の文字クラス判定群（is_digit/is_amount 等）は
 * Perl パーサー側（scripts/dt）の字句処理用で、ホスト汎用ツールキットの
 * 対象外（reference 扱い）。使用箇所が現れた工程で追加する。
 *
 * is_check_idx の >= 0 判定は「エラーコードは負の long」という原典 32bit
 * の不変条件に依存する。LP64 での成立は result.h の DEVEL_ERR_ 符号拡張が
 * 保証する（result.h 冒頭の注記を参照）。
 *********************************************/
#include "types.h"

inline bool is_check(unsigned long _result){ return (_result > 0); }
inline bool is_check(long _result){ return (_result > 0); }
inline bool is_check(bool _bool){ return _bool; }
inline bool is_check(void* _ref){ return (_ref != 0); }
inline bool is_check_idx(idx_ _idx){ return (_idx >= 0); }

#endif /* CPP_DEVEL_COND_H */

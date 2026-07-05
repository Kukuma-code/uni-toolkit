#ifndef CPP_DEVEL_A_APRN_H
#define CPP_DEVEL_A_APRN_H
/*********************************************
 * 出力層: 自由関数 p() 群 + newline()（工程4 で移植）
 *
 * 原典: include/a.h の p() オーバーロード群（101-124行）と
 * include/cpp_pre.h:1 の newline()。uni__/element__ 等の p()/info()
 * メンバと各クラスの自由関数 p(const X&, status_) がここへ委譲する。
 *
 * 原典からの現代化:
 *   - putln/putn/putuln/putd（自前 printf 群＝ベアメタル libc 代替）は
 *     ホストの printf 書式に置換。数値の桁揃え等の体裁は原典と異なり得るが
 *     デバッグ出力であり意味論（値・区切り・改行・null ガード）は保存する
 *   - INLINE マクロ -> inline
 *   - 第2引数 _idx はスカラ版では位置合わせ専用（原典から不使用）
 *
 * オーバーロードの原典仕様（そのまま保存）:
 *   - p(char*, idx, status) は idx に依らず文字列全体を %s で出力する
 *     （1文字出力は範囲版 p(char*,qty,begin,end) か uni__::p(idx) を使う）
 *   - 範囲版は begin/end を [0, qty) にクランプし、end < 0 は末尾からの
 *     相対位置。要素間セパレータは status & FORMAT::MASK の文字
 *     （char 範囲版はセパレータなしの連続出力）
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "../format.h"

inline void newline(status_ _status = FORMAT::NEWLINE){ if ( FORMAT::NEWLINE & _status ) putchar('\n'); }

/* --- スカラ --- */
inline void p(bool _bool, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%d", _bool); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(double _d, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%f", _d); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(char _ch, idx_ _idx = 0, status_ _status = 0){ (void)_idx; putchar(_ch); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(long _long, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%ld", _long); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(unsigned long _long, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%lu", _long); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(int _int, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%d", _int); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(unsigned int _int, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%u", _int); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(long long _long, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%lld", _long); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(unsigned long long _long, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%llu", _long); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }

/* --- 配列要素（null ガード付き。_idx は既定なし＝スカラ版との弁別） --- */
inline void p(bool* _bool, idx_ _idx, status_ _status = 0){ if ( _bool == 0 ) printf("[Null Pointer]"); else printf("%d", _bool[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(double* _d, idx_ _idx, status_ _status = 0){ if ( _d == 0 ) printf("[Null Pointer]"); else printf("%f", _d[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(long* _long, idx_ _idx, status_ _status = 0){ if ( _long == 0 ) printf("[Null Pointer]"); else printf("%ld", _long[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(unsigned long* _ulong, idx_ _idx, status_ _status = 0){ if ( _ulong == 0 ) printf("[Null Pointer]"); else printf("%lu", _ulong[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(int* _int, idx_ _idx, status_ _status = 0){ if ( _int == 0 ) printf("[Null Pointer]"); else printf("%d", _int[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(unsigned int* _uint, idx_ _idx, status_ _status = 0){ if ( _uint == 0 ) printf("[Null Pointer]"); else printf("%u", _uint[_idx]); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }

/* 文字列は idx に依らず全体を出力（原典仕様） */
inline void p(char* _str, idx_ _idx = 0, status_ _status = 0){ (void)_idx; if ( _str == 0 ) printf("[Null Pointer]"); else printf("%s", _str); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }
inline void p(void* _ptr, idx_ _idx = 0, status_ _status = 0){ (void)_idx; printf("%p", _ptr); if ( _status & FORMAT::NEWLINE ) putchar('\n'); }

/* --- 範囲（char: セパレータなし連続出力） --- */
inline void p(char* _str, qty_ _qty, idx_ _begin, idx_ _end, status_ _status = FORMAT::SEPARATOR){
	if ( _begin < 0 ) _begin = 0;
	if ( _begin >= _qty ) _begin = _qty - 1;
	if ( _end >= _qty ) _end = _qty - 1;
	if ( _end < 0 ) _end = _qty - 1 + _end;
	if ( _begin > _end ) _end = _begin;
	for ( idx_ i = _begin; i <= _end; ++i ){ putchar(_str[i]); }
	if ( _status & FORMAT::NEWLINE ) putchar('\n');
}

/* --- 範囲（総称: 要素間に status & MASK のセパレータ） --- */
template <typename A>
inline void p(A* _ref, qty_ _qty, idx_ _begin, idx_ _end, status_ _status = FORMAT::SEPARATOR){
	if ( _begin < 0 ) _begin = 0;
	if ( _begin >= _qty ) _begin = _qty - 1;
	if ( _end >= _qty ) _end = _qty - 1;
	if ( _end < 0 ) _end = _qty - 1 + _end;
	if ( _begin > _end ) _end = _begin;
	for ( idx_ i = _begin; i <= _end; ++i ){
		::p(_ref[i]);
		if ( i < _end ) putchar((int)(_status & FORMAT::MASK));
	}
	if ( _status & FORMAT::NEWLINE ) putchar('\n');
}

#endif /* CPP_DEVEL_A_APRN_H */

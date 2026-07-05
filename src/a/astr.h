#ifndef CPP_DEVEL_A_ASTR_H
#define CPP_DEVEL_A_ASTR_H
/*********************************************
 * a 層: C 文字列プリミティブ（原典 include/a.h の inline 版が SSOT）。
 *
 * 原典 lib/a の .c 群は C リンク用の別実装で、C++ テンプレート層が使うのは
 * include/a.h の inline 群。ここではそのうち reference_init / operator_base
 * が呼ぶ 4 つだけを移植する（参照引数版・acat 等は使用箇所が現れた工程で）。
 *
 * 意味論は原典どおり:
 *   alen  - NUL までの長さ（strlen 相当）
 *   acpy  - NUL まで複製し NUL を書く。複製した文字数を返す
 *   ancpy - 最大 cnt 文字複製（src の NUL で停止）、最後に必ず _dst[i]=0。
 *           cnt 文字コピーした場合 _dst[cnt] に書くため、呼び出し側は
 *           cnt+1 バイト確保していること（init_qty の char +1 確保と対）
 *   acmp  - 一致しない最初の位置の差（strcmp 同型。0 = 等しい）
 *********************************************/
#include "../types.h"

inline unsigned int alen(const char* src){
	size_ ref; for ( ref = 0; src[ref]; ++ref ){} return (unsigned int)ref;
}

inline qty_ acpy(const void* _src, char* _dst){
	unsigned long tmpnum = 0;
	while ( ((const char*)_src)[tmpnum] != 0 ){
		_dst[tmpnum] = ((const char*)_src)[tmpnum];
		tmpnum = tmpnum + 1;
	}
	_dst[tmpnum] = 0;
	return (qty_)tmpnum;
}

inline void ancpy(const void* src, char* _dst, size_ cnt){
	size_ i = 0;
	while ( (i < cnt) && (_dst[i] = ((const char*)src)[i]) ) ++i;
	_dst[i] = 0;
}

inline int acmp(const char* src, const char* dst){
	int ret = 0;
	while ( src[ret] != 0 && src[ret] == dst[ret] ) ++ret;
	ret = src[ret] - dst[ret];
	return ret;
}

#endif /* CPP_DEVEL_A_ASTR_H */

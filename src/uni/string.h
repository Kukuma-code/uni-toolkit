#ifndef CPP_DEVEL_UNI_STRING_H
#define CPP_DEVEL_UNI_STRING_H
/*********************************************
 * string_ = uni__<char, char*>（原典 backup/tmp_include/string.h）
 *
 * uni_ 基盤の char 実体化そのもの。welf.c の唯一残る重い依存であり、
 * この typedef の成立が本プロジェクト分離の理由（from_cpp_triage.md §1）。
 *
 * p() は原典どおりの自己完結プリンタ（printf/putchar/alen のみ依存）。
 * 原典の `if(printf(...));` の空 if は除去。
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "../format.h"
#include "../a/astr.h"
#include "uni.h"

typedef uni__<char, char*> uni_string_;
typedef uni__<char, char*> string_;

inline void p(string_* _str, idx_ _idx = -1, status_ _status = 0){
	if ( _str == 0 ){ printf("[Null Class Pointer]"); }
	else {
		if ( (*_str)() == 0 ){ printf("[Null Pointer]"); }
		else {
			if ( _idx == -1 ){ printf("%s", (*_str)()); }
			else {
				qty_ len = alen((*_str)());
				if ( _idx < 0 || len - 1 < _idx ){ printf("[Out of Range]"); }
				else { printf("%c", (*_str)()[_idx]); }
			}
		}
	}
	if ( _status & FORMAT::NEWLINE ){ putchar('\n'); }
}

#endif /* CPP_DEVEL_UNI_STRING_H */

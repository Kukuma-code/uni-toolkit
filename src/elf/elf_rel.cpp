/*********************************************
 * elf_rel_ / elf_rela_ のダンプ実体（原典 lib/elf/elf_rel.c）
 * 現代化方針は elf_ehdr.cpp と同一。
 *********************************************/
#include <cstdio>
#include "elf_rel.h"

void elf_rel_::info(){
#define TMP_PRINT(x) printf("%-16s:[0x%llx]\n", #x, (unsigned long long)(x))
	TMP_PRINT(ofs);
	TMP_PRINT(type);
	TMP_PRINT(sym);
#undef TMP_PRINT
}

void elf_rela_::info(){
#define TMP_PRINT(x) printf("%-16s:[0x%llx]\n", #x, (unsigned long long)(x))
	TMP_PRINT(ofs);
	TMP_PRINT(type);
	TMP_PRINT(sym);
	TMP_PRINT(addend);
#undef TMP_PRINT
}

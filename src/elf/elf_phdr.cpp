/*********************************************
 * elf_phdr_ のダンプ実体（原典 lib/elf/elf_phdr.c）
 * 現代化方針は elf_ehdr.cpp と同一（平文メンバ・%llx・presettings 除去）。
 * token 表は原典と同一（tk_flags が TK_END を欠くのも原典どおりだが、
 * print_flags は表引きせずビット分解するため到達しない。表は未使用のまま
 * 保存し、is_token_name には渡さないこと）。
 *********************************************/
#include <cstdio>
#include "misc_token.h"
#include "elf_types.h"
#include "elf_phdr.h"

namespace ELF{
	namespace PHDR{
#define TKP(x) TK(PT_##x)
		token_ tk_type[]={TKP(NULL),TKP(LOAD),TKP(DYNAMIC),TKP(INTERP),TKP(NOTE),
		                  TKP(SHLIB),TKP(PHDR),TKP(TLS),TKP(NUM),TKP(LOOS),
		                  TKP(GNU_EH_FRAME),TKP(GNU_STACK),TKP(GNU_RELRO),
		                  TKP(LOSUNW),TKP(SUNWBSS),TKP(SUNWSTACK),
		                  TKP(HISUNW),TKP(HIOS),
		                  TKP(LOPROC),TKP(HIPROC),TK_END()};
#undef TKP
		token_ tk_flags[]={TK(PF_X),TK(PF_W),TK(PF_R)};
	}
}

void elf_phdr_::print_flags(){
	elf_u32_ f = this->flags;
	if ( f >= PF_R ){ printf("[read]");  f -= PF_R; }
	if ( f >= PF_W ){ printf("[write]"); f -= PF_W; }
	if ( f >= PF_X ){ printf("[exec]");  f -= PF_X; }
}

const char* elf_phdr_::is_typename(){ return is_token_name(type, ELF::PHDR::tk_type); }

void elf_phdr_::info(){
#define TMP_PRINT(x)  printf("\t%-16s:[%x:%s]\n", #x, (unsigned int)(x), is_token_name((long)(x), ELF::PHDR::tk_##x))
#define TMP_PRINT2(x) printf("\t%-16s:[%llx]\n", #x, (unsigned long long)(x))
	TMP_PRINT(type);
	TMP_PRINT2(ofs);
	TMP_PRINT2(vaddr);
	TMP_PRINT2(paddr);
	TMP_PRINT2(file_size);
	TMP_PRINT2(mem_size);
	printf("\t%-16s:%x:", "flags", flags);
	print_flags();
	printf("\n");
	TMP_PRINT2(align);
#undef TMP_PRINT
#undef TMP_PRINT2
}

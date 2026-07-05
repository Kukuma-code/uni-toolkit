/*********************************************
 * elf_shdr_ のダンプ実体（原典 lib/elf/elf_shdr.c）
 * 現代化方針は elf_ehdr.cpp と同一。token 表は原典と同一。
 *********************************************/
#include <cstdio>
#include "misc_token.h"
#include "elf_types.h"
#include "elf_shdr.h"

namespace ELF{
	namespace SHDR{
#define TKS(x) TK(SHT_##x)
		token_ tk_type[]={TKS(NULL),TKS(PROGBITS),TKS(SYMTAB),TKS(STRTAB),TKS(RELA),TKS(HASH),
		                  TKS(DYNAMIC),TKS(NOTE),TKS(NOBITS),TKS(REL),TKS(SHLIB),TKS(DYNSYM),
		                  TKS(INIT_ARRAY),TKS(FINI_ARRAY),TKS(PREINIT_ARRAY),TKS(GROUP),
		                  TKS(SYMTAB_SHNDX),TKS(RELR),TKS(NUM),TKS(LOOS),TKS(GNU_ATTRIBUTES),
		                  TKS(GNU_HASH),TKS(GNU_LIBLIST),TKS(CHECKSUM),TKS(LOSUNW),
		                  TKS(SUNW_move),TKS(SUNW_COMDAT),TKS(SUNW_syminfo),TKS(GNU_verdef),
		                  TKS(GNU_verneed),TKS(GNU_versym),TKS(HISUNW),TKS(HIOS),
		                  TKS(LOPROC),TKS(HIPROC),TKS(LOUSER),TKS(HIUSER),TK_END()};
#undef TKS
		token_ tk_flags[]={TK(SHF_WRITE),TK(SHF_ALLOC),TK(SHF_EXECINSTR),TK(SHF_MASKPROC),TK_END()};
	}
}

elf_shdr_::elf_shdr_(){}

const char* elf_shdr_::is_typename(){ return is_token_name(type, ELF::SHDR::tk_type); }

void elf_shdr_::print_flags(){
#define CHECK_SHF(x) if ( flags & SHF_##x ){ printf("[" #x "]"); }
	CHECK_SHF(WRITE); CHECK_SHF(ALLOC); CHECK_SHF(EXECINSTR); CHECK_SHF(MERGE);
	CHECK_SHF(STRINGS); CHECK_SHF(INFO_LINK); CHECK_SHF(LINK_ORDER); CHECK_SHF(OS_NONCONFORMING);
	CHECK_SHF(GROUP); CHECK_SHF(TLS);
#undef CHECK_SHF
}

void elf_shdr_::info(){
#define TMP_PRINT(x)  printf("\t%-16s:[0x%x:%s]\n", #x, (unsigned int)(x), is_token_name((long)(x), ELF::SHDR::tk_##x))
#define TMP_PRINT2(x) printf("\t%-16s:[0x%llx]\n", #x, (unsigned long long)(x))
	TMP_PRINT2(name);
	TMP_PRINT(type);
	printf("\t%-16s:[0x%llx]:", "flags", (unsigned long long)flags);
	print_flags();
	printf("\n");
	TMP_PRINT2(addr);
	TMP_PRINT2(ofs);
	TMP_PRINT2(size);
	TMP_PRINT2(link);
	TMP_PRINT2(_info);
	TMP_PRINT2(addr_align);
	TMP_PRINT2(entsize);
#undef TMP_PRINT
#undef TMP_PRINT2
}

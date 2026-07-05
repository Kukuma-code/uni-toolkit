/*********************************************
 * elf_dyn_ のダンプ実体（原典 lib/elf/elf_dyn.c）
 * 現代化方針は elf_ehdr.cpp と同一。tag ごとの val/ptr 表示分岐は原典どおり。
 *********************************************/
#include <cstdio>
#include "elf_dyn.h"
#include "misc_token.h"

namespace ELF{
	namespace DYN{
#define TKD(x) TK(DT_##x)
		token_ tk_tag[]={TKD(NULL),TKD(NEEDED),TKD(PLTRELSZ),TKD(PLTGOT),
		                 TKD(HASH),TKD(STRTAB),TKD(SYMTAB),TKD(RELA),
		                 TKD(RELASZ),TKD(RELAENT),TKD(STRSZ),TKD(SYMENT),
		                 TKD(INIT),TKD(FINI),TKD(SONAME),TKD(RPATH),TKD(SYMBOLIC),
		                 TKD(REL),TKD(RELSZ),TKD(RELENT),TKD(PLTREL),TKD(DEBUG),
		                 TKD(TEXTREL),TKD(JMPREL),TKD(BIND_NOW),TKD(INIT_ARRAY),
		                 TKD(FINI_ARRAY),TKD(INIT_ARRAYSZ),TKD(FINI_ARRAYSZ),TKD(LOOS),
		                 TKD(HIOS),TKD(LOPROC),TKD(HIPROC),TK_END()};
#undef TKD
	}
}

void elf_dyn_::info(){
#define TMP_PRINT(x) printf("%-16s:[0x%llx]\n", #x, (unsigned long long)(x))
	printf("%-16s:[0x%llx:%s]\n", "tag", (unsigned long long)tag, is_token_name(tag, ELF::DYN::tk_tag));
	switch(tag){
	case DT_NEEDED: case DT_PLTRELSZ: case DT_RELASZ: case DT_RELAENT: case DT_STRSZ:
	case DT_SYMENT: case DT_SONAME: case DT_RPATH: case DT_RELSZ: case DT_RELENT:
	case DT_PLTREL:
		TMP_PRINT(val);
		break;
	case DT_PLTGOT: case DT_HASH: case DT_STRTAB: case DT_SYMTAB:
	case DT_RELA: case DT_INIT: case DT_FINI: case DT_REL: case DT_DEBUG:
	case DT_JMPREL: case DT_INIT_ARRAY: case DT_FINI_ARRAY:
	case DT_INIT_ARRAYSZ: case DT_FINI_ARRAYSZ:
		TMP_PRINT(ptr);
		break;
	case DT_NULL: case DT_SYMBOLIC: case DT_TEXTREL: case DT_BIND_NOW:
	case DT_LOOS: case DT_HIOS: case DT_LOPROC: case DT_HIPROC:
		break;
	}
#undef TMP_PRINT
}

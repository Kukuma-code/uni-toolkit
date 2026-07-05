/*********************************************
 * elf_sym_ のダンプ実体（原典 lib/elf/elf_sym.c）
 * 現代化方針は elf_ehdr.cpp と同一。
 *
 * 原典由来のバグ修正（bug_log id=97。cpp_devarm と同一の修正・原作者承認済み）:
 *   原典の tk_type / tk_bind は TK_END() 番兵を欠き、表に無い type/bind
 *   （例: STT の 5..9,11,14）で is_token_name が value 一致も value==-1 も
 *   見つけられず配列外読込になる。両表末尾に TK_END() を追加（該当値は
 *   "UNKNOWN"）。表は tests から検証できるよう extern 公開（elf_sym.h）。
 *********************************************/
#include <cstdio>
#include "elf_types.h"
#include "misc_token.h"
#include "elf_sym.h"

namespace ELF{
	namespace SYM{
		token_ tk_type[]={TK(STT_NOTYPE),TK(STT_OBJECT),TK(STT_FUNC),TK(STT_SECTION),
		                  TK(STT_FILE),TK(STT_LOOS),TK(STT_HIOS),TK(STT_LOPROC),TK(STT_HIPROC),
		                  TK_END()};
		token_ tk_bind[]={TK(STB_LOCAL),TK(STB_GLOBAL),TK(STB_WEAK),TK(STB_LOOS),TK(STB_HIOS),
		                  TK(STB_LOPROC),TK(STB_HIPROC),TK_END()};
	}
}

void elf_sym_::info(){
	printf("name:0x%x\n", name);
	printf("type:0x%x:%s\n", type, is_token_name(type, ELF::SYM::tk_type));
	printf("bind:0x%x:%s\n", bind, is_token_name(bind, ELF::SYM::tk_bind));
	printf("other:0x%x\n", other);
	printf("shdr_ndx:0x%x\n", shdr_ndx);
	printf("value:0x%llx\n", (unsigned long long)value);
	printf("size:0x%llx\n", (unsigned long long)size);
}

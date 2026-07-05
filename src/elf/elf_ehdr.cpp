/*********************************************
 * elf_ehdr_ のダンプ実体（原典 lib/elf/elf_ehdr.c）
 *
 * 原典からの現代化:
 *   - THIS_FUNC マクロ -> 平文メンバ定義、<presettings.h> 依存の除去
 *   - 64bit フィールド（entry/phdr_ofs/shdr_ofs）は %llx キャストで完全表示
 *     （原典は %x で下位 32bit のみ。libc printf は -Wformat 検査があるため
 *     幅も正す。cpp_devarm は自前 printf のため %x のまま＝表示幅のみの差）
 *   - raw_data の puth（自前 16 進出力）-> printf("%02x")
 * token 表は原典と同一。
 *********************************************/
#include <cstdio>
#include "../types.h"
#include "misc_token.h"
#include "elf_types.h"
#include "elf_ehdr.h"

namespace ELF{
	namespace EHDR{
		token_ tk_elf_class[] ={TK(ELFCLASSNONE),TK(ELFCLASS32),TK(ELFCLASS64),TK_END()};
		token_ tk_byte_order[]={TK(ELFDATANONE),TK(ELFDATA2LSB),TK(ELFDATA2MSB),TK_END()};
		token_ tk_version[]={TK(EV_NONE),TK(EV_CURRENT),TK_END()};
		token_ tk_type[]={TK(ET_NONE),TK(ET_REL),TK(ET_EXEC),TK(ET_DYN),TK(ET_CORE),TK_END()};
		token_ tk_machine[]={TK(EM_NONE),TK(EM_M32),TK(EM_SPARC),TK(EM_386),TK(EM_68K),TK(EM_88K),
		                     TK(EM_860),TK(EM_MIPS),
		                     TK(EM_PARISC),TK(EM_SPARC32PLUS),TK(EM_PPC),TK(EM_ALPHA),TK(EM_SPARCV9),
		                     TK(EM_VAX),TK(EM_X86_64),
		                     TK(EM_ARM),TK(EM_AARCH64),TK(EM_RISCV),   /* 現行仕様の追補（工程6） */
		                     TK_END()};
	}
}

void elf_ehdr_::info(){
	printf("IDENT(\n");
	printf("\tMAGIC:");
	for ( size_ i = 0; i <= EI_MAG3; i++ ){ printf(" ['%c' %x]", ident[i], ident[i]); }
	printf("\n");
#define TMP_PRINT(x)  printf("\t%-16s:[0x%x:%s]\n", #x, (unsigned int)(x), is_token_name((long)(x), ELF::EHDR::tk_##x))
#define TMP_PRINT2(x) printf("\t%-16s:[0x%llx]\n", #x, (unsigned long long)(x))
	TMP_PRINT(elf_class);
	TMP_PRINT(byte_order);
	printf("\t%-16s:[0x%x:%s]\n", "elf_version", ident_version, is_token_name(ident_version, ELF::EHDR::tk_version));
	TMP_PRINT2(os_abi);
	TMP_PRINT2(abi_version);
	printf(")\n");
	TMP_PRINT(type);
	TMP_PRINT(machine);
	TMP_PRINT(version);
	TMP_PRINT2(entry);
	TMP_PRINT2(phdr_ofs);
	TMP_PRINT2(shdr_ofs);
	TMP_PRINT2(flags);
	TMP_PRINT2(ehdr_size);
	TMP_PRINT2(phdr_entsize);
	TMP_PRINT2(phdr_qty);
	TMP_PRINT2(shdr_entsize);
	TMP_PRINT2(shdr_qty);
	TMP_PRINT2(shdr_strndx);
#undef TMP_PRINT
#undef TMP_PRINT2
}

void elf_ehdr_::raw_data(){
	qty_ qty = sizeof(elf_ehdr_);
	for ( qty_ i = 0; i < qty; ++i ){
		putchar(i % 16 ? ' ' : '\n');
		printf("%02x", ((unsigned char*)this)[i]);
	}
	putchar('\n');
}

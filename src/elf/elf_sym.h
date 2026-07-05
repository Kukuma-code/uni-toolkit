#ifndef CPP_DEVEL_ELF_ELF_SYM_H
#define CPP_DEVEL_ELF_ELF_SYM_H
/*********************************************
 * elf_sym_ — シンボル表エントリ（原典 include/elf_sym.h）
 * レイアウトは ELF64（Elf64_Sym, 24 bytes）。_info の bitfield は
 * little-endian で低 4bit=type / 高 4bit=bind（ELF64_ST_TYPE/BIND 対応）。
 *
 * token 表 tk_type / tk_bind は tests から直接検証できるよう extern 公開
 * する（原典は .c 内限定。bug_log id=97 の TK_END 修正の検証点）。
 *********************************************/
#include "elf_types.h"
#include "misc_token.h"

class elf_sym_ {
 public:
	elf_u32_ name;                 /* 名前（strtab オフセット） */
	union {
		unsigned char _info;       /* type + bind */
		struct {
			unsigned char type : 4;
			unsigned char bind : 4;
		};
	} __attribute__((__packed__));
	unsigned char other;           /* 可視性 */
	elf_u16_  shdr_ndx;            /* 所属セクション */
	elf_addr_ value;
	elf_u64_  size;
 public:
	void info();
} __attribute__((__packed__));

namespace ELF { namespace SYM {
	extern token_ tk_type[];
	extern token_ tk_bind[];
} }

#endif /* CPP_DEVEL_ELF_ELF_SYM_H */

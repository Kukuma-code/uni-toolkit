#ifndef CPP_DEVEL_ELF_ELF_SHDR_H
#define CPP_DEVEL_ELF_ELF_SHDR_H
/*********************************************
 * elf_shdr_ — セクションヘッダ（原典 include/elf_shdr.h）
 * レイアウトは ELF64（Elf64_Shdr, 64 bytes）。
 *********************************************/
#include "elf_types.h"

class elf_shdr_ {
 public:
	elf_u32_  name;
	elf_u32_  type;
	elf_u64_  flags;
	elf_addr_ addr;
	elf_ofs_  ofs;
	elf_u64_  size;
	elf_u32_  link;
	elf_u32_  _info;
	elf_u64_  addr_align;
	elf_u64_  entsize;
 public:
	elf_shdr_();
	const char* is_typename();
	void print_flags();
	void info();
};

#endif /* CPP_DEVEL_ELF_ELF_SHDR_H */

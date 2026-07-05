#ifndef CPP_DEVEL_ELF_ELF_PHDR_H
#define CPP_DEVEL_ELF_ELF_PHDR_H
/*********************************************
 * elf_phdr_ — プログラムヘッダ（原典 include/elf_phdr.h）
 * レイアウトは ELF64（Elf64_Phdr, 56 bytes。flags が 2 番目に来る
 * ELF64 のフィールド順）。
 *********************************************/
#include "elf_types.h"

class elf_phdr_ {
 public:
	elf_u32_  type;
	elf_u32_  flags;
	elf_ofs_  ofs;
	elf_addr_ vaddr;
	elf_addr_ paddr;
	elf_u64_  file_size;
	elf_u64_  mem_size;
	elf_u64_  align;
 public:
	const char* is_typename();
	void print_flags();
	void info();
};

#endif /* CPP_DEVEL_ELF_ELF_PHDR_H */

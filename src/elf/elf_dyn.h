#ifndef CPP_DEVEL_ELF_ELF_DYN_H
#define CPP_DEVEL_ELF_ELF_DYN_H
/*********************************************
 * elf_dyn_ — 動的リンク情報エントリ（原典 include/elf_dyn.h）
 * レイアウトは ELF64（Elf64_Dyn, 16 bytes）。
 *********************************************/
#include "elf_types.h"

class elf_dyn_ {
 public:
	elf_s64_ tag;
	union {
		elf_u64_ val;
		elf_u64_ ptr;
	};
 public:
	void info();
};

#endif /* CPP_DEVEL_ELF_ELF_DYN_H */

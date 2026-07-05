#ifndef CPP_DEVEL_ELF_ELF_REL_H
#define CPP_DEVEL_ELF_ELF_REL_H
/*********************************************
 * elf_rel_ / elf_rela_ — 再配置エントリ（原典 include/elf_rel.h）
 * レイアウトは ELF64（Elf64_Rel=16 bytes / Elf64_Rela=24 bytes）。
 * _info の union 分解は little-endian で低 32bit=type / 高 32bit=sym
 * （ELF64_R_TYPE/R_SYM 対応。原典どおり）。
 *********************************************/
#include "elf_types.h"

class elf_rel_ {
 public:
	elf_addr_ ofs;
	union {
		elf_u64_ _info;
		struct {
			elf_u32_ type;
			elf_u32_ sym;
		};
	};
 public:
	void info();
};

class elf_rela_ {
 public:
	elf_addr_ ofs;
	union {
		elf_u64_ _info;
		struct {
			elf_u32_ type;
			elf_u32_ sym;
		};
	};
	elf_s64_ addend;
 public:
	void info();
};

#endif /* CPP_DEVEL_ELF_ELF_REL_H */

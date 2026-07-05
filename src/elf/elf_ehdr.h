#ifndef CPP_DEVEL_ELF_ELF_EHDR_H
#define CPP_DEVEL_ELF_ELF_EHDR_H
/*********************************************
 * elf_ehdr_ — ELF ファイルヘッダ（原典 include/elf_ehdr.h）
 *
 * レイアウトは ELF64（Elf64_Ehdr, 64 bytes）。ident 先頭 16 バイトは
 * union で名前付きフィールドに重ねる（匿名 struct は clang/gcc 拡張。
 * 原典どおり）。DECLCLASS(CLASSNAME,0) は非テンプレートの素のクラス宣言
 * だったため、平文の class へ展開して macro 世界を落とす（modernize）。
 *********************************************/
#include "elf_types.h"

class elf_ehdr_ {
 public:
	union {
		unsigned char ident[EI_NIDENT];
		struct {
			char magic[4];
			char elf_class;
			char byte_order;
			char ident_version;
			char os_abi;
			char abi_version;
			char _pad[7];
		} __attribute__((__packed__));
	};
	elf_u16_  type;
	elf_u16_  machine;
	elf_u32_  version;
	elf_addr_ entry;
	elf_ofs_  phdr_ofs;
	elf_ofs_  shdr_ofs;
	elf_u32_  flags;
	elf_u16_  ehdr_size;
	elf_u16_  phdr_entsize;
	elf_u16_  phdr_qty;
	elf_u16_  shdr_entsize;
	elf_u16_  shdr_qty;
	elf_u16_  shdr_strndx;

	void info();
	void raw_data();
};

#endif /* CPP_DEVEL_ELF_ELF_EHDR_H */

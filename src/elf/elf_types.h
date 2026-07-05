#ifndef CPP_DEVEL_ELF_ELF_TYPES_H
#define CPP_DEVEL_ELF_ELF_TYPES_H
/*********************************************
 * ELF の幅型 + 仕様定数（可搬・ELF64 基準）
 *
 * 原典 include/elf_types.h は
 *   #ifdef ARM_ELF #include <linux/elf.h> #else #include <elf.h>
 * でシステム ELF ヘッダに依存し、addr/ofs/64bit 幅を unsigned long
 * （データモデル依存）で持っていた（ILP32->ELF32, LP64->ELF64 と偶然
 * 一致する脆い設計）。
 *
 * 本実装は cpp_devarm core/elf/elf_types.h（同じ devel 由来の host 移植・
 * 照合済み参照実装）と同一方針:
 *   - システム <elf.h> 依存を除去し <cstdint> の固定幅型へ。
 *     elf_addr_/elf_ofs_ = uint64_t で ELF64 レイアウト固定
 *     （Elf64_Ehdr=64B に一致。工程6 で実データ検証）
 *   - dumper の token 表が参照する ELF 仕様定数を vendoring。
 *     値は ELF 仕様 / glibc <elf.h> 準拠。名前重複（LOSUNW==SUNW_move 等）
 *     は原典どおり
 *
 * 工程6 検証（2026-07-02）: 本ファイルの全 #define 定数を musl の
 * /usr/include/elf.h（alpine, docker 経由）と機械照合し全一致を確認。
 * 唯一の乖離は SHT_NUM で、原典時代の 19 から現行仕様では SHT_RELR=19 が
 * 追加され NUM=20 に更新されている -> 現行仕様に追随。あわせて現代の
 * machine 値（EM_ARM/EM_AARCH64/EM_RISCV）を追補（実バイナリ検証用）。
 *********************************************/
#include <cstdint>

/* --- 幅型（ELF64 固定） --- */
typedef uint16_t elf_u16_;
typedef uint32_t elf_u32_;
typedef uint64_t elf_u64_;
typedef int32_t  elf_s32_;
typedef int64_t  elf_s64_;
typedef uint64_t elf_addr_;   /* ELF64: 8 bytes */
typedef uint64_t elf_ofs_;    /* ELF64: 8 bytes */

/* 抽象 ELF 要素種別（原典どおり。elf_kind 等が参照） */
namespace ELF {
namespace TYPE {
enum type {
	BYTE = 0, ADDR, DYN, EHDR, HALF, OFF, PHDR, RELA, REL, SHDR, SWORD, SYM, WORD,
	SXWORD, XWORD,
	VDEF, VNEED,
	NUM        /* must be last */
};
}
}
typedef ELF::TYPE::type elf_type_;

/* --- e_ident --- */
#define EI_NIDENT 16
#define EI_MAG3   3

/* --- ELF class (e_ident[EI_CLASS]) --- */
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

/* --- data encoding (e_ident[EI_DATA]) --- */
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/* --- file version --- */
#define EV_NONE    0
#define EV_CURRENT 1

/* --- object file type (e_type) --- */
#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

/* --- machine (e_machine) --- */
#define EM_NONE        0
#define EM_M32         1
#define EM_SPARC       2
#define EM_386         3
#define EM_68K         4
#define EM_88K         5
#define EM_860         7
#define EM_MIPS        8
#define EM_PARISC      15
#define EM_SPARC32PLUS 18
#define EM_PPC         20
#define EM_ALPHA       0x9026   /* glibc の Linux 値（非標準番号。musl と一致確認済み） */
#define EM_SPARCV9     43
#define EM_X86_64      62
#define EM_VAX         75
/* 現行仕様の追補（原典表に無い現代アーキテクチャ。工程6） */
#define EM_ARM         40
#define EM_AARCH64     183
#define EM_RISCV       243

/* --- section type (sh_type) --- */
#define SHT_NULL          0
#define SHT_PROGBITS      1
#define SHT_SYMTAB        2
#define SHT_STRTAB        3
#define SHT_RELA          4
#define SHT_HASH          5
#define SHT_DYNAMIC       6
#define SHT_NOTE          7
#define SHT_NOBITS        8
#define SHT_REL           9
#define SHT_SHLIB         10
#define SHT_DYNSYM        11
#define SHT_INIT_ARRAY    14
#define SHT_FINI_ARRAY    15
#define SHT_PREINIT_ARRAY 16
#define SHT_GROUP         17
#define SHT_SYMTAB_SHNDX  18
#define SHT_RELR          19           /* 現行仕様の追補（原典時代は NUM=19） */
#define SHT_NUM           20
#define SHT_LOOS          0x60000000
#ifndef SHT_GNU_ATTRIBUTES
#define SHT_GNU_ATTRIBUTES 0x6ffffff5
#endif
#define SHT_GNU_HASH      0x6ffffff6
#define SHT_GNU_LIBLIST   0x6ffffff7
#define SHT_CHECKSUM      0x6ffffff8
#define SHT_LOSUNW        0x6ffffffa
#define SHT_SUNW_move     0x6ffffffa
#define SHT_SUNW_COMDAT   0x6ffffffb
#define SHT_SUNW_syminfo  0x6ffffffc
#define SHT_GNU_verdef    0x6ffffffd
#define SHT_GNU_verneed   0x6ffffffe
#define SHT_GNU_versym    0x6fffffff
#define SHT_HISUNW        0x6fffffff
#define SHT_HIOS          0x6fffffff
#define SHT_LOPROC        0x70000000
#define SHT_HIPROC        0x7fffffff
#define SHT_LOUSER        0x80000000
#define SHT_HIUSER        0x8fffffff

/* --- section flags (sh_flags) --- */
#define SHF_WRITE            0x1
#define SHF_ALLOC            0x2
#define SHF_EXECINSTR        0x4
#define SHF_MERGE            0x10
#define SHF_STRINGS          0x20
#define SHF_INFO_LINK        0x40
#define SHF_LINK_ORDER       0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP            0x200
#define SHF_TLS              0x400
#define SHF_MASKPROC         0xf0000000

/* --- program header type (p_type) --- */
#define PT_NULL         0
#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_SHLIB        5
#define PT_PHDR         6
#define PT_TLS          7
#define PT_NUM          8
#define PT_LOOS         0x60000000
#define PT_GNU_EH_FRAME 0x6474e550
#define PT_GNU_STACK    0x6474e551
#define PT_GNU_RELRO    0x6474e552
#define PT_LOSUNW       0x6ffffffa
#define PT_SUNWBSS      0x6ffffffa
#define PT_SUNWSTACK    0x6ffffffb
#define PT_HISUNW       0x6fffffff
#define PT_HIOS         0x6fffffff
#define PT_LOPROC       0x70000000
#define PT_HIPROC       0x7fffffff

/* --- program header flags (p_flags) --- */
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

/* --- dynamic tag (d_tag) --- */
#define DT_NULL         0
#define DT_NEEDED       1
#define DT_PLTRELSZ     2
#define DT_PLTGOT       3
#define DT_HASH         4
#define DT_STRTAB       5
#define DT_SYMTAB       6
#define DT_RELA         7
#define DT_RELASZ       8
#define DT_RELAENT      9
#define DT_STRSZ        10
#define DT_SYMENT       11
#define DT_INIT         12
#define DT_FINI         13
#define DT_SONAME       14
#define DT_RPATH        15
#define DT_SYMBOLIC     16
#define DT_REL          17
#define DT_RELSZ        18
#define DT_RELENT       19
#define DT_PLTREL       20
#define DT_DEBUG        21
#define DT_TEXTREL      22
#define DT_JMPREL       23
#define DT_BIND_NOW     24
#define DT_INIT_ARRAY   25
#define DT_FINI_ARRAY   26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_LOOS         0x6000000d
#define DT_HIOS         0x6ffff000
#define DT_LOPROC       0x70000000
#define DT_HIPROC       0x7fffffff

/* --- symbol type (ELF*_ST_TYPE) --- */
#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_LOOS    10
#define STT_HIOS    12
#define STT_LOPROC  13
#define STT_HIPROC  15

/* --- symbol binding (ELF*_ST_BIND) --- */
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2
#define STB_LOOS   10
#define STB_HIOS   12
#define STB_LOPROC 13
#define STB_HIPROC 15

#endif /* CPP_DEVEL_ELF_ELF_TYPES_H */

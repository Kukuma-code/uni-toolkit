#ifndef CPP_DEVEL_ELF_ELF_KIND_H
#define CPP_DEVEL_ELF_ELF_KIND_H
/* ELF 要素の大分類（原典 include/elf_kind.h。welf ctor の第3引数） */

namespace ELF {
	namespace KIND { enum type { NONE, AR, COFF, STD, NUM }; }
	typedef enum KIND::type kind_;
}
typedef ELF::kind_ elf_kind_;

#endif /* CPP_DEVEL_ELF_ELF_KIND_H */

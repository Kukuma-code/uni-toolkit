#ifndef CPP_DEVEL_ELF_ELF_VERSION_H
#define CPP_DEVEL_ELF_ELF_VERSION_H
/* ELF バージョン種別（原典 include/elf_version.h。welf の version 管理） */

namespace ELF {
	namespace VERSION { enum type { NONE, CURRENT, NUM }; }
	typedef enum VERSION::type version_;
}
typedef ELF::version_ elf_version_;

#endif /* CPP_DEVEL_ELF_ELF_VERSION_H */

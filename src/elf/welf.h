#ifndef CPP_DEVEL_ELF_WELF_H
#define CPP_DEVEL_ELF_WELF_H
/*********************************************
 * elf_ — ELF リーダ本体（原典 include/welf.h + lib/elf/welf.c）
 *
 * ファイルを開いて ehdr / phdr[] / shdr[] を読み込み、セクション名を
 * string_（uni_ 基盤）の配列に取り込む。show_* が対話ダンプの中核。
 * 本プロジェクト分離の理由である「welf の string_ 依存」の現物。
 *
 * 原典からの現代化:
 *   - throw(std::bad_ctor) 動的例外仕様の除去、std::bad_ctor -> ::bad_ctor
 *   - <presettings.h> / syscall.h 依存を除去し、POSIX は <fcntl.h> /
 *     <unistd.h> を直接使う（ACC_R=O_RDONLY, SEEK_BEGIN=SEEK_SET。値も同一）
 *   - DECLCLASS / THIS_FUNC マクロ -> 平文クラス
 *   - ctor 途中失敗時に確保済みメンバと fd を解放してから throw する
 *     （原典は open 後の throw で fd・ehdr 等がリーク）
 *   - ehdr の検証を追加: ELF マジック / ELFCLASS64 / phdr・shdr の
 *     entsize がレイアウトと一致 / shdr_strndx が範囲内 / strtab 終端 NUL
 *   - check_phdr_qty / check_shdr_qty の引数を size_（unsigned、0<= が
 *     恒真）から idx_ に修正
 *
 * 原典由来のバグ修正（詳細は welf.cpp 冒頭）:
 *   - phdr 読込前に phdr_ofs へ lseek しない（ehdr 直後レイアウト前提）
 *   - セクション名表を new[]（＝アリーナ）で確保し、その内部ポインタを
 *     string_ に代入 -> uni_ の共有プロトコル（アリーナ内は refcnt 共有）が
 *     ノード先頭でないポインタの mnode ヘッダを踏んで壊す
 *   - rel_info / rela_info の read が sizeof(elf_sym_) 固定（elf_rel_ は
 *     16 バイトのため 8 バイトのヒープあふれ）
 *   - interactive の添字入力を範囲検査の前に配列参照する
 *********************************************/
/* welf.h は原典どおり elf クラスタの集約ヘッダ（利用側はこれ 1 本を
   include する）。直接参照しないものも意図的に re-export する */
#include <fcntl.h>
#include "../types.h"
#include "../exception.h"    // IWYU pragma: export
#include "../uni/string.h"
#include "misc_token.h"      // IWYU pragma: export
#include "elf_types.h"       // IWYU pragma: export
#include "elf_kind.h"
#include "elf_version.h"
#include "elf_phdr.h"
#include "elf_ehdr.h"
#include "elf_shdr.h"
#include "elf_sym.h"         // IWYU pragma: export
#include "elf_rel.h"         // IWYU pragma: export
#include "elf_dyn.h"         // IWYU pragma: export

class elf_ {
 public:
	elf_ehdr_* ehdr;
	elf_phdr_* phdr;
	elf_shdr_* shdr;
	string_*   shdr_name;
	int fd;
	elf_version_ version;
 public:
	elf_(const char* _name, const int _access = O_RDONLY, ELF::kind_ _kind = ELF::KIND::STD);
	~elf_();
	result_ check_phdr_qty(idx_ _idx){ return (0 <= _idx && _idx < ehdr->phdr_qty); }
	result_ check_shdr_qty(idx_ _idx){ return (0 <= _idx && _idx < ehdr->shdr_qty); }
	void version_init();
	elf_version_ version_is(elf_version_ _version);
	elf_version_ is_version();
	void info();
	void rel_info(int _no, char* _buf);
	void rela_info(int _no, char* _buf);
	void strtab_info(int _no);
	void symtab_info(int _no, char* _buf);
	int symtab_message_info(qty_ _qty);
	void show_phdr(idx_ _idx = -1);
	void show_shdr(idx_ _idx = -1);
	void pt_dynamic_info(int _no, char* _buf);
	void show();
	void interactive();
	long interactive_info();
 private:
	void cleanup();
};

#endif /* CPP_DEVEL_ELF_WELF_H */

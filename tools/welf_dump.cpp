/*********************************************
 * welf_dump — elf_ リーダの非対話ダンプ CLI（工程6 実データ検証用）
 *
 * 原典 welf の interactive() は stdin 駆動のため自動照合に向かない。
 * 同じ elf_ を使って ehdr / phdr / shdr を一括表示する薄い CLI を用意し、
 * 実 ELF64 バイナリの解析結果を llvm-objdump 等と突き合わせる。
 *
 * 使い方: welf_dump <elf64-file> [ehdr|phdr|shdr|all]（既定 all）
 *********************************************/
#include <cstdio>
#include "a/astr.h"
#include "elf/welf.h"

int main(int argc, char** argv){
	if ( argc < 2 ){
		printf("usage: welf_dump <elf64-file> [ehdr|phdr|shdr|all]\n");
		return 2;
	}
	const char* mode = (argc >= 3) ? argv[2] : "all";
	try {
		elf_ e(argv[1]);
		bool all = (acmp(mode, "all") == 0);
		if ( all || acmp(mode, "ehdr") == 0 ){
			printf("==== EHDR ====\n");
			e.ehdr->info();
		}
		if ( all || acmp(mode, "phdr") == 0 ){
			printf("==== PHDR x%d ====\n", e.ehdr->phdr_qty);
			e.show_phdr();
		}
		if ( all || acmp(mode, "shdr") == 0 ){
			printf("==== SHDR x%d ====\n", e.ehdr->shdr_qty);
			e.show_shdr();
		}
	} catch (const bad_ctor&){
		printf("welf_dump: failed to open/parse <%s>\n", argv[1]);
		return 1;
	}
	return 0;
}

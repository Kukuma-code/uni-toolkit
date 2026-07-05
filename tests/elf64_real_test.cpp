/*********************************************
 * 工程6 検証: 実 ELF64 データの照合（密閉テスト）
 *
 * fixtures/real_elf.c を clang の cross target（x86_64-unknown-linux-gnu）
 * でコンパイルした本物の ELF64 relocatable をビルド時に生成し、welf で
 * 解析して独立プロデューサ（clang の ELF エミッタ）の出力と突き合わせる。
 *
 * 観測対象:
 *   1. ehdr: ET_REL / EM_X86_64 / phdr なし
 *   2. セクション: .text（PROGBITS, ALLOC|EXECINSTR）と .symtab
 *      （SYMTAB, entsize == sizeof(elf_sym_)）が名前解決できる
 *   3. シンボル表の実読: add = STT_FUNC/STB_GLOBAL、global_var =
 *      STT_OBJECT/サイズ4 を strtab 名で特定できる
 *
 * 実行ファイル（phdr あり）の実データ照合は busybox（alpine, x86-64 と
 * aarch64）で実施済み: shdr 23/23・phdr 12/12・9/9 の全フィールドが
 * llvm-objdump と一致（2026-07-02, 工程6 コミットメッセージ参照）。
 * docker 依存のため ctest には含めない。
 *********************************************/
#include <cassert>
#include <cstdio>
#include <unistd.h>
#include "elf/welf.h"
#include "a/astr.h"

namespace {

idx_ find_section(elf_& e, const char* nm){
	for ( idx_ i = 0; i < (idx_)e.ehdr->shdr_qty; ++i ){
		if ( e.shdr_name[i]() != 0 && acmp(e.shdr_name[i](), nm) == 0 ) return i;
	}
	return -1;
}

}

int main(){
	elf_ e("real_elf64.o");

	/* --- 1. ehdr --- */
	assert(e.ehdr->type == ET_REL);
	assert(e.ehdr->machine == EM_X86_64);
	assert(e.ehdr->phdr_qty == 0);
	assert(e.ehdr->shdr_qty > 0);

	/* --- 2. セクション名の解決と属性 --- */
	idx_ text_i = find_section(e, ".text");
	assert(text_i > 0);
	assert(e.shdr[text_i].type == SHT_PROGBITS);
	assert((e.shdr[text_i].flags & (SHF_ALLOC | SHF_EXECINSTR)) == (SHF_ALLOC | SHF_EXECINSTR));
	assert(e.shdr[text_i].size > 0);

	idx_ sym_i = find_section(e, ".symtab");
	assert(sym_i > 0);
	assert(e.shdr[sym_i].type == SHT_SYMTAB);
	assert(e.shdr[sym_i].entsize == sizeof(elf_sym_));
	assert(e.shdr[sym_i].link < e.ehdr->shdr_qty);

	/* --- 3. シンボル表の実読（e.fd を直接使う） --- */
	elf_shdr_& symtab = e.shdr[sym_i];
	elf_shdr_& strtab = e.shdr[symtab.link];
	qty_ sym_qty = (qty_)(symtab.size / symtab.entsize);
	assert(sym_qty >= 3);                        /* null + add + global_var（+ファイル名等） */

	bool found_add = false, found_gvar = false;
	for ( qty_ i = 1; i < sym_qty; ++i ){
		elf_sym_ s;
		lseek(e.fd, (off_t)(symtab.ofs + symtab.entsize * (size_)i), SEEK_SET);
		{
			long r = (long)read(e.fd, &s, sizeof(s));
			assert(r == (long)sizeof(s));
		}
		if ( s.name == 0 ) continue;
		char nm[64];
		lseek(e.fd, (off_t)(strtab.ofs + s.name), SEEK_SET);
		{
			long r = (long)read(e.fd, nm, sizeof(nm) - 1);
			assert(r > 0);
			nm[r] = 0;
		}
		if ( acmp(nm, "add") == 0 ){
			assert(s.type == STT_FUNC && s.bind == STB_GLOBAL);
			assert(s.size > 0);
			assert(acmp(is_token_name(s.type, ELF::SYM::tk_type), "STT_FUNC") == 0);
			found_add = true;
		}
		if ( acmp(nm, "global_var") == 0 ){
			assert(s.type == STT_OBJECT && s.size == 4);
			found_gvar = true;
		}
	}
	assert(found_add && found_gvar);

	printf("elf64_real_test: all assertions passed\n");
	return 0;
}

/*********************************************
 * 工程5 検証: elf クラスタ（welf リーダ + ダンパ + token 表）
 *
 * 観測対象:
 *   1. 構造体レイアウトが ELF64 仕様サイズと一致（static_assert）
 *   2. elf_sym_ / elf_rel_ の union ビット割り（type/bind, type/sym）
 *   3. tk_type / tk_bind の TK_END 番兵（bug_log id=97 修正）:
 *      表に無い値で "UNKNOWN"、表内の値で正しい名前
 *   4. 合成 ELF64 ファイルの読み込み:
 *      - phdr 表が ehdr 直後に無い配置（bug1 の lseek 修正の検証）
 *      - セクション名が string_ へ深コピーされる（bug2 修正の検証:
 *        名前ノードがアリーナ内・refcnt=1・内容一致）
 *      - check_phdr_qty / check_shdr_qty の境界
 *   5. 不正入力で bad_ctor: 存在しないパス / 非 ELF / ELFCLASS32
 *   6. dtor でセクション名ノードがアリーナへ返る
 *   7. show_phdr / show_shdr の煙試験（出力は目視）
 *
 * 合成 ELF はこのテストが自前で組み立てて書き出す（host は Mach-O のため
 * システムに ELF 実データが無い。実バイナリとの照合は工程6）。
 *********************************************/
#include <cassert>
#include <cstdio>
#include <cstring>
#include "elf/welf.h"
#include "a/astr.h"
#include "mem/mnode.h"
#include "mem/mallocator.h"

/* --- 1. ELF64 仕様サイズ --- */
static_assert(sizeof(elf_ehdr_) == 64, "Elf64_Ehdr size");
static_assert(sizeof(elf_phdr_) == 56, "Elf64_Phdr size");
static_assert(sizeof(elf_shdr_) == 64, "Elf64_Shdr size");
static_assert(sizeof(elf_sym_)  == 24, "Elf64_Sym size");
static_assert(sizeof(elf_rel_)  == 16, "Elf64_Rel size");
static_assert(sizeof(elf_rela_) == 24, "Elf64_Rela size");
static_assert(sizeof(elf_dyn_)  == 16, "Elf64_Dyn size");

namespace {

const char* ELF_PATH     = "welf_test.elf";
const char* GARBAGE_PATH = "welf_test_garbage.bin";
const char* ELF32_PATH   = "welf_test_elf32.bin";

/* 合成 ELF64 の配置（phdr 表を ehdr 直後に置かないのが要点） */
enum : unsigned long {
	OFS_PHDR     = 0x100,
	OFS_SHSTRTAB = 0x200,
	OFS_STRTAB   = 0x300,
	OFS_SYMTAB   = 0x340,
	OFS_RELA     = 0x3a0,
	OFS_SHDR     = 0x400,
	FILE_SIZE    = 0x580
};

void build_elf_file(){
	static unsigned char image[FILE_SIZE];
	memset(image, 0, sizeof(image));

	elf_ehdr_ eh;
	memset(&eh, 0, sizeof(eh));
	eh.magic[0] = 0x7f; eh.magic[1] = 'E'; eh.magic[2] = 'L'; eh.magic[3] = 'F';
	eh.elf_class     = ELFCLASS64;
	eh.byte_order    = ELFDATA2LSB;
	eh.ident_version = EV_CURRENT;
	eh.type          = ET_EXEC;
	eh.machine       = EM_X86_64;
	eh.version       = EV_CURRENT;
	eh.entry         = 0x400000;
	eh.phdr_ofs      = OFS_PHDR;
	eh.shdr_ofs      = OFS_SHDR;
	eh.ehdr_size     = sizeof(elf_ehdr_);
	eh.phdr_entsize  = sizeof(elf_phdr_);
	eh.phdr_qty      = 1;
	eh.shdr_entsize  = sizeof(elf_shdr_);
	eh.shdr_qty      = 6;
	eh.shdr_strndx   = 1;
	memcpy(&image[0], &eh, sizeof(eh));

	elf_phdr_ ph;
	memset(&ph, 0, sizeof(ph));
	ph.type = PT_LOAD; ph.flags = PF_R | PF_X;
	ph.ofs = 0; ph.vaddr = 0x400000; ph.paddr = 0x400000;
	ph.file_size = FILE_SIZE; ph.mem_size = FILE_SIZE; ph.align = 0x1000;
	memcpy(&image[OFS_PHDR], &ph, sizeof(ph));

	/* .shstrtab: 0:"" 1:".shstrtab" 11:".symtab" 19:".strtab" 27:".rela.text" 38:".text" */
	const char shstr[] = "\0.shstrtab\0.symtab\0.strtab\0.rela.text\0.text";
	memcpy(&image[OFS_SHSTRTAB], shstr, sizeof(shstr));   /* sizeof は終端 NUL 込み 44 */

	/* .strtab: 0:"" 1:"main" 6:"foo" */
	const char str[] = "\0main\0foo";
	memcpy(&image[OFS_STRTAB], str, sizeof(str));

	/* .symtab: [0]null [1]main=STT_FUNC/STB_GLOBAL [2]foo=type6(表外)/STB_LOCAL */
	elf_sym_ syms[3];
	memset(syms, 0, sizeof(syms));
	syms[1].name = 1; syms[1].type = STT_FUNC;  syms[1].bind = STB_GLOBAL;
	syms[1].shdr_ndx = 5; syms[1].value = 0x400010; syms[1].size = 0x20;
	syms[2].name = 6; syms[2].type = 6;         syms[2].bind = STB_LOCAL;
	syms[2].shdr_ndx = 5; syms[2].value = 0x400040; syms[2].size = 0x8;
	memcpy(&image[OFS_SYMTAB], syms, sizeof(syms));

	elf_rela_ rela;
	memset(&rela, 0, sizeof(rela));
	rela.ofs = 0x10; rela.type = 1; rela.sym = 1; rela.addend = -4;
	memcpy(&image[OFS_RELA], &rela, sizeof(rela));

	elf_shdr_ sh[6];
	memset(sh, 0, sizeof(sh));
	sh[1].name = 1;  sh[1].type = SHT_STRTAB;   sh[1].ofs = OFS_SHSTRTAB; sh[1].size = sizeof(shstr);
	sh[2].name = 11; sh[2].type = SHT_SYMTAB;   sh[2].ofs = OFS_SYMTAB;   sh[2].size = sizeof(syms);
	sh[2].link = 3;  sh[2].entsize = sizeof(elf_sym_);
	sh[3].name = 19; sh[3].type = SHT_STRTAB;   sh[3].ofs = OFS_STRTAB;   sh[3].size = sizeof(str);
	sh[4].name = 27; sh[4].type = SHT_RELA;     sh[4].ofs = OFS_RELA;     sh[4].size = sizeof(rela);
	sh[4].link = 2;  sh[4]._info = 5;           sh[4].entsize = sizeof(elf_rela_);
	sh[5].name = 38; sh[5].type = SHT_PROGBITS; sh[5].ofs = 0x500;        sh[5].size = 0x20;
	sh[5].flags = SHF_ALLOC | SHF_EXECINSTR;
	memcpy(&image[OFS_SHDR], sh, sizeof(sh));

	FILE* fp = fopen(ELF_PATH, "wb");
	assert(fp != nullptr);
	{
		size_t written = fwrite(image, 1, sizeof(image), fp);
		assert(written == sizeof(image));
	}
	fclose(fp);
}

void build_bad_files(){
	FILE* fp = fopen(GARBAGE_PATH, "wb");
	assert(fp != nullptr);
	{
		const char junk[128] = "this is not an elf file";
		size_t written = fwrite(junk, 1, sizeof(junk), fp);
		assert(written == sizeof(junk));
	}
	fclose(fp);

	/* magic は正しいが ELFCLASS32 */
	unsigned char e32[64];
	memset(e32, 0, sizeof(e32));
	e32[0] = 0x7f; e32[1] = 'E'; e32[2] = 'L'; e32[3] = 'F';
	e32[4] = ELFCLASS32;
	fp = fopen(ELF32_PATH, "wb");
	assert(fp != nullptr);
	{
		size_t written = fwrite(e32, 1, sizeof(e32), fp);
		assert(written == sizeof(e32));
	}
	fclose(fp);
}

}

int main(){
	/* --- 2. union のビット割り（little-endian） --- */
	{
		elf_sym_ s;
		memset(&s, 0, sizeof(s));
		s._info = 0x21;                        /* 低 4bit=type, 高 4bit=bind */
		assert(s.type == 1 && s.bind == 2);
		elf_rel_ r;
		r._info = 0x0000000500000001ULL;       /* 低 32bit=type, 高 32bit=sym */
		assert(r.type == 1 && r.sym == 5);
	}

	/* --- 3. token 表: TK_END 番兵（bug_log id=97） --- */
	assert(acmp(is_token_name(STT_FUNC, ELF::SYM::tk_type), "STT_FUNC") == 0);
	assert(acmp(is_token_name(6, ELF::SYM::tk_type), "UNKNOWN") == 0);    /* 表外 */
	assert(acmp(is_token_name(14, ELF::SYM::tk_type), "UNKNOWN") == 0);
	assert(acmp(is_token_name(STB_WEAK, ELF::SYM::tk_bind), "STB_WEAK") == 0);
	assert(acmp(is_token_name(9, ELF::SYM::tk_bind), "UNKNOWN") == 0);    /* 表外 */

	build_elf_file();
	build_bad_files();

	/* --- 4. 合成 ELF64 の読み込み --- */
	char* name_node = nullptr;
	{
		elf_ e(ELF_PATH);
		assert(e.ehdr->machine == EM_X86_64);
		assert(e.ehdr->type == ET_EXEC);
		assert(e.ehdr->phdr_qty == 1 && e.ehdr->shdr_qty == 6);

		/* phdr は ehdr 直後ではなく 0x100 に置いてある（bug1 修正の検証） */
		assert(e.phdr[0].type == PT_LOAD);
		assert(e.phdr[0].vaddr == 0x400000);
		assert(acmp(e.phdr[0].is_typename(), "PT_LOAD") == 0);

		/* セクション名: 深コピーされた string_（bug2 修正の検証） */
		assert(acmp(e.shdr_name[0](), "null") == 0);
		assert(acmp(e.shdr_name[1](), ".shstrtab") == 0);
		assert(acmp(e.shdr_name[2](), ".symtab") == 0);
		assert(acmp(e.shdr_name[3](), ".strtab") == 0);
		assert(acmp(e.shdr_name[4](), ".rela.text") == 0);
		assert(acmp(e.shdr_name[5](), ".text") == 0);
		name_node = e.shdr_name[2]();
		assert(mallocator.is_memory(name_node));
		assert(mnode_refcnt(name_node) == 1);

		assert(acmp(e.shdr[2].is_typename(), "SHT_SYMTAB") == 0);
		assert(acmp(e.shdr[4].is_typename(), "SHT_RELA") == 0);

		/* 境界 */
		assert(e.check_phdr_qty(0) && !e.check_phdr_qty(1) && !e.check_phdr_qty(-1));
		assert(e.check_shdr_qty(5) && !e.check_shdr_qty(6) && !e.check_shdr_qty(-1));

		/* --- 7. ダンプの煙試験（体裁は目視。クラッシュしないこと） --- */
		printf("--- show_phdr(0) ---\n");
		e.show_phdr(0);
		printf("--- show_shdr(2) ---\n");
		e.show_shdr(2);
		printf("--- show_shdr(99) ---\n");
		e.show_shdr(99);

		assert(e.is_version() == ELF::VERSION::NONE);
		e.version_init();
		assert(e.is_version() == ELF::VERSION::CURRENT);
	}
	/* --- 6. dtor: 名前ノードがアリーナへ返る --- */
	assert(!(mnode_header(name_node)->state & MALLOCATOR::USED));

	/* --- 5. 不正入力は bad_ctor --- */
	{
		bool caught = false;
		try { elf_ e("no_such_file.elf"); }
		catch (const bad_ctor&){ caught = true; }
		assert(caught);
	}
	{
		bool caught = false;
		try { elf_ e(GARBAGE_PATH); }
		catch (const bad_ctor&){ caught = true; }
		assert(caught);
	}
	{
		bool caught = false;
		try { elf_ e(ELF32_PATH); }
		catch (const bad_ctor&){ caught = true; }
		assert(caught);
	}

	remove(ELF_PATH);
	remove(GARBAGE_PATH);
	remove(ELF32_PATH);
	printf("welf_test: all assertions passed\n");
	return 0;
}

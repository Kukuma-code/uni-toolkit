/*********************************************
 * elf_ の実体（原典 lib/elf/welf.c）
 *
 * 原典由来のバグ修正（bug: 印。設計注記は welf.h 冒頭）:
 *   bug1: ctor が phdr 読込前に phdr_ofs へ lseek しない。ehdr 直後に
 *         phdr 表が続く典型レイアウトだけで動いていた。lseek を追加
 *   bug2: セクション名の一時バッファを new[]（グローバル new 差し替えで
 *         アリーナ確保）にし、その内部ポインタ &buf[ofs] を string_ に代入
 *         していた。uni_ の init(char**,const char*) はアリーナ内ポインタを
 *         refcnt 共有するため、ノード先頭でない内部ポインタの「mnode
 *         ヘッダ」（実体は名前表の文字列バイト）を読み書きして壊れる。
 *         バッファを malloc（非アリーナ）にして必ず深コピー経路に固定
 *   bug3: rel_info / rela_info の read が symtab_info の写しのまま
 *         sizeof(elf_sym_)=24 固定。elf_rel_ は 16 バイトで 8 バイトの
 *         ヒープあふれ。各構造体の sizeof に修正
 *   bug4: interactive が入力添字を範囲検査の前に phdr[ret] / shdr[ret] で
 *         参照（任意入力で OOB 読み）。検査を先行させる
 *   bug5: symtab_info の名前読みで read の戻り値を検査せず _buf[ret]=0
 *         （read 失敗 ret=-1 で _buf[-1] 書き）。ret>0 のみ処理に修正
 *   bug6: strtab_info の length 検証が len でなく直前の read 戻り値 ret を
 *         見ていた（コピペ由来）。len に修正
 *
 * その他の現代化:
 *   - 対話サブルーチンの elf_dyn_/elf_rel_/elf_rela_/elf_sym_ の new を
 *     delete で解放（原典はリーク）
 *   - aton は conv クラスタ（reference 扱い）から本ファイル限定の static
 *     関数として最小移植（interactive の数値入力にのみ必要）
 *   - read エラー時の exit(-1) は原典どおり（対話ツールの中断挙動）
 *********************************************/
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "welf.h"

/* --- aton 最小移植（原典 lib/conv/aton.c。基数接頭辞 0x/0b/0d/0o 対応） --- */
namespace {
int aton(char* str, int* ret, char sep = '\n'){
	unsigned char ch, radix, sgn = 0;
	*ret = 0; radix = 10;
	if ( (ch = (unsigned char)*str) == '-' ){ sgn = 1; ++str; }
	if ( alen(str) > 2 ){
		switch(str[1]){
		case 'x': case 'X': radix = 16; str = (str + 2); break;
		case 'b': case 'B': radix = 2;  str = (str + 2); break;
		case 'd': case 'D': radix = 10; str = (str + 2); break;
		case 'o': case 'O': radix = 8;  str = (str + 2); break;
		default: break;
		}
	}
	while ( (ch = (unsigned char)*str++) != 0 && ch != (unsigned char)sep ){
		ch -= 0x30;
		if ( ch >= 0x31 ) ch -= 0x20;
		if ( ch >= 0x11 ) ch -= 0x07;
		if ( (radix == 16 && ch <= 0x0f) || (radix != 16 && ch <= 9) ){ *ret = *ret * radix + ch; }
		else return NUL_(ATON_INVALID_CH);
	}
	if ( sgn ) *ret = -*ret;
	return TRUE;
}
}

elf_::elf_(const char* _name, const int _access, const ELF::kind_ _kind)
	: ehdr(0), phdr(0), shdr(0), shdr_name(0), fd(-1), version(ELF::VERSION::NONE){
	(void)_kind;   /* 原典から不使用（KIND 分岐は未実装のまま保存） */
	fd = open(_name, _access, S_IRUSR);
	if ( fd == -1 ){ printf("can't open <%s>\n", _name); throw bad_ctor(); }
	ehdr = new elf_ehdr_;
	size_ r_width = sizeof(elf_ehdr_);
	qty_ qty;
	if ( (qty = read(fd, ehdr, r_width)) != (qty_)r_width ){
		printf("read error.\n"); cleanup(); throw bad_ctor();
	}
	/* 検証（modernize）: ELF64 レイアウト前提の成立を ctor で保証する */
	if ( !(ehdr->magic[0] == 0x7f && ehdr->magic[1] == 'E' &&
	       ehdr->magic[2] == 'L' && ehdr->magic[3] == 'F') ){
		printf("not an ELF file <%s>\n", _name); cleanup(); throw bad_ctor();
	}
	if ( ehdr->elf_class != ELFCLASS64 ){
		printf("not ELFCLASS64 <%s>\n", _name); cleanup(); throw bad_ctor();
	}
	if ( (ehdr->phdr_qty != 0 && ehdr->phdr_entsize != sizeof(elf_phdr_)) ||
	     (ehdr->shdr_qty != 0 && ehdr->shdr_entsize != sizeof(elf_shdr_)) ||
	     (ehdr->shdr_qty != 0 && !(ehdr->shdr_strndx < ehdr->shdr_qty)) ){
		printf("broken ELF header <%s>\n", _name); cleanup(); throw bad_ctor();
	}

	lseek(fd, (off_t)ehdr->phdr_ofs, SEEK_SET);   /* bug1 修正 */
	phdr = new elf_phdr_[ehdr->phdr_qty];
	qty_ limit = this->ehdr->phdr_qty;
	for ( qty_ i = 0; i < limit; ++i ){
		if ( (qty = read(fd, &phdr[i], ehdr->phdr_entsize)) != ehdr->phdr_entsize ){
			printf("read error.\n"); cleanup(); throw bad_ctor();
		}
	}

	lseek(fd, (off_t)ehdr->shdr_ofs, SEEK_SET);
	shdr = new elf_shdr_[ehdr->shdr_qty];
	limit = this->ehdr->shdr_qty;
	for ( qty_ i = 0; i < limit; ++i ){
		if ( (qty = read(fd, &shdr[i], ehdr->shdr_entsize)) != ehdr->shdr_entsize ){
			printf("read error.\n"); cleanup(); throw bad_ctor();
		}
	}

	/* セクション名表。bug2 修正: バッファは malloc（非アリーナ）で確保し、
	   string_ への代入を必ず深コピー経路にする */
	elf_shdr_* tmp = &shdr[ehdr->shdr_strndx];
	lseek(fd, (off_t)tmp->ofs, SEEK_SET);
	unsigned char* buf = (unsigned char*)malloc(tmp->size);
	if ( buf == 0 ){ cleanup(); throw bad_ctor(); }
	if ( (qty = read(fd, buf, tmp->size)) != (qty_)tmp->size ){
		free(buf); cleanup(); throw bad_ctor();
	}
	if ( tmp->size == 0 || buf[tmp->size - 1] != 0 ){   /* strtab 終端 NUL 検証 */
		free(buf); cleanup(); throw bad_ctor();
	}
	shdr_name = new string_[ehdr->shdr_qty];
	qty = ehdr->shdr_qty;
	shdr_name[0] = "null";
	for ( qty_ i = 1; i < qty; ++i ){
		if ( shdr[i].name < tmp->size ){ shdr_name[i] = (char*)&buf[shdr[i].name]; }
		else { shdr_name[i] = "null"; }   /* 名前オフセット破損の防御（modernize） */
	}
	free(buf);
}

void elf_::cleanup(){
	delete ehdr; delete [] phdr; delete [] shdr; delete [] shdr_name;
	ehdr = 0; phdr = 0; shdr = 0; shdr_name = 0;
	if ( fd != -1 ){ close(fd); fd = -1; }
}

elf_::~elf_(){ cleanup(); }

void elf_::version_init(){ this->version = ELF::VERSION::CURRENT; }
elf_version_ elf_::version_is(elf_version_ _version){ return this->version = _version; }
elf_version_ elf_::is_version(){ return this->version; }

void elf_::info(){
}

void elf_::show_phdr(idx_ _idx){
	if ( _idx == -1 ){
		for ( elf_u16_ i = 0; i < ehdr->phdr_qty; i++ ){ printf("PHDR %u\n", i); phdr[i].info(); }
	} else {
		if ( this->check_phdr_qty(_idx) ){ printf("PHDR %ld\n", _idx); phdr[_idx].info(); }
		else { printf("[OUT OF RANGE]:range is PHDR[0...%d]\n", ehdr->phdr_qty - 1); }
	}
}

void elf_::show_shdr(idx_ _idx){
	if ( _idx == -1 ){
		for ( elf_u16_ i = 1; i < ehdr->shdr_qty; i++ ){ printf("SHDR %u:%s\n", i, shdr_name[i]()); shdr[i].info(); }
	} else {
		if ( this->check_shdr_qty(_idx) ){ printf("SHDR %ld:%s\n", _idx, shdr_name[_idx]()); shdr[_idx].info(); }
		else { printf("[OUT OF RANGE]:range is SHDR[0...%d]\n", ehdr->shdr_qty - 1); }
	}
}

void elf_::show(){}

void elf_::interactive(){
#define BUFLIMIT 64
	char buf[BUFLIMIT];
	for ( int i = 0; i < BUFLIMIT; ++i ) buf[i] = 0;   /* 原典 buf_init */
	int ret;
	while ( interactive_info() && (ret = (int)read(0, buf, BUFLIMIT - 1)) > 0 ){
		buf[ret] = 0;
		aton(buf, &ret);
		switch(ret){
		case 1:
			ehdr->info(); break;
		case 2:
			for ( long i = 0; i < ehdr->phdr_qty; ++i ){ printf("%ld:%s\n", i, phdr[i].is_typename()); }
			printf("phdr no?:\n");
			ret = (int)read(0, buf, BUFLIMIT - 1);
			if ( ret <= 0 ) break;
			buf[ret] = 0;
			aton(buf, &ret, '\n');
			if ( !this->check_phdr_qty(ret) ){ this->show_phdr(ret); break; }   /* bug4 修正: 検査を先行 */
			if ( phdr[ret].type == PT_DYNAMIC ){ pt_dynamic_info(ret, buf); }
			else { printf("[%d]\n", ret); this->show_phdr(ret); }
			break;
		case 3:
			for ( long i = 0; i < ehdr->shdr_qty; ++i ){ printf("%02ld:[%-32s]:%s\n", i, shdr_name[i](), shdr[i].is_typename()); }
			printf("shdr no?:\n");
			ret = (int)read(0, buf, BUFLIMIT - 1);
			if ( ret <= 0 ) break;
			buf[ret] = 0;
			aton(buf, &ret, '\n');
			if ( !this->check_shdr_qty(ret) ){ this->show_shdr(ret); break; }   /* bug4 修正 */
			if ( shdr[ret].type == SHT_SYMTAB ){ symtab_info(ret, buf); }
			else if ( shdr[ret].type == SHT_STRTAB ){ strtab_info(ret); }
			else if ( shdr[ret].type == SHT_REL ){ rel_info(ret, buf); }
			else if ( shdr[ret].type == SHT_RELA ){ rela_info(ret, buf); }
			else { this->show_shdr(ret); }
			break;
		}
	}
}

void elf_::pt_dynamic_info(int _no, char* _buf){
	int ret;
	elf_dyn_* ref = new elf_dyn_;
	this->show_phdr(_no);
	qty_ sym_qty = (qty_)(phdr[_no].file_size / sizeof(elf_dyn_)) - 1;
	while ( symtab_message_info(sym_qty) && (ret = (int)read(0, _buf, BUFLIMIT - 1)) > 0 ){
		_buf[ret] = 0;
		aton(_buf, &ret, '\n');
		if ( ret < 0 || ret > sym_qty ){ printf("[OUT of RANGE]\n"); continue; }
		lseek(fd, (off_t)(phdr[_no].ofs + sizeof(elf_dyn_) * (size_)ret), SEEK_SET);
		if ( (ret = (int)read(fd, ref, sizeof(elf_dyn_))) != sizeof(elf_dyn_) ){
			printf("read error\n"); exit(-1);
		}
		ref->info();
	}
	delete ref;
}

int elf_::symtab_message_info(qty_ _qty){ printf("[0-%ld]\n", _qty); return TRUE; }

void elf_::rel_info(int _no, char* _buf){
	int ret;
	elf_rel_* ref = new elf_rel_;
	qty_ sym_qty = (qty_)(shdr[_no].size / shdr[_no].entsize) - 1;
	this->show_shdr(_no);
	while ( symtab_message_info(sym_qty) && (ret = (int)read(0, _buf, BUFLIMIT - 1)) > 0 ){
		_buf[ret] = 0;
		aton(_buf, &ret, '\n');
		if ( ret < 0 || ret > sym_qty ){ printf("[OUT of RANGE]\n"); continue; }
		lseek(fd, (off_t)(shdr[_no].ofs + shdr[_no].entsize * (size_)ret), SEEK_SET);
		if ( (ret = (int)read(fd, ref, sizeof(elf_rel_))) != (int)sizeof(elf_rel_) ){   /* bug3 修正 */
			printf("read error\n"); exit(-1);
		}
		ref->info();
	}
	delete ref;
}

void elf_::rela_info(int _no, char* _buf){
	int ret;
	elf_rela_* ref = new elf_rela_;
	qty_ sym_qty = (qty_)(shdr[_no].size / shdr[_no].entsize) - 1;
	this->show_shdr(_no);
	while ( symtab_message_info(sym_qty) && (ret = (int)read(0, _buf, BUFLIMIT - 1)) > 0 ){
		_buf[ret] = 0;
		aton(_buf, &ret, '\n');
		if ( ret < 0 || ret > sym_qty ){ printf("[OUT of RANGE]\n"); continue; }
		lseek(fd, (off_t)(shdr[_no].ofs + shdr[_no].entsize * (size_)ret), SEEK_SET);
		if ( (ret = (int)read(fd, ref, sizeof(elf_rela_))) != (int)sizeof(elf_rela_) ){   /* bug3 修正 */
			printf("read error\n"); exit(-1);
		}
		ref->info();
	}
	delete ref;
}

void elf_::symtab_info(int _no, char* _buf){
	int ret;
	elf_sym_* ref = new elf_sym_;
	qty_ sym_qty = (qty_)(shdr[_no].size / shdr[_no].entsize) - 1;
	this->show_shdr(_no);
	while ( symtab_message_info(sym_qty) && (ret = (int)read(0, _buf, BUFLIMIT - 1)) > 0 ){
		_buf[ret] = 0;
		aton(_buf, &ret, '\n');
		if ( ret < 0 || ret > sym_qty ){ printf("[OUT of RANGE]\n"); continue; }
		lseek(fd, (off_t)(shdr[_no].ofs + shdr[_no].entsize * (size_)ret), SEEK_SET);
		if ( (ret = (int)read(fd, ref, sizeof(elf_sym_))) != (int)shdr[_no].entsize ){
			printf("read error\n"); exit(-1);
		}
		if ( ref->name != 0 && shdr[_no].link < ehdr->shdr_qty ){
			lseek(fd, (off_t)(shdr[shdr[_no].link].ofs + ref->name), SEEK_SET);
			ret = (int)read(fd, _buf, sizeof(elf_sym_));
			if ( ret > 0 ){ _buf[ret] = 0; printf("%s\n", _buf); }   /* bug5 修正 */
		}
		ref->info();
	}
	delete ref;
}

void elf_::strtab_info(int _no){
	char buf[512];
	int len; int ret, width, ofs;
	this->show_shdr(_no);
	printf("[offset?]\n");
	ofs = (int)read(0, buf, BUFLIMIT - 1);
	if ( ofs < 0 ) return;
	buf[ofs] = 0;
	aton(buf, &ofs, '\n');
	if ( ofs < 0 || (elf_u64_)ofs > shdr[_no].size ){ ofs = 0; }
	lseek(fd, (off_t)(shdr[_no].ofs + (size_)ofs), SEEK_SET);
	printf("[length?]\n");
	ret = (int)read(0, buf, BUFLIMIT - 1);
	if ( ret < 0 ) return;
	buf[ret] = 0;
	aton(buf, &len, '\n');
	if ( len <= 0 || (elf_u64_)len > shdr[_no].size ){ len = (int)shdr[_no].size; }   /* bug6 修正 */
	ofs = (int)shdr[_no].size - ofs;
	if ( ofs <= len ){ len = ofs; }
	while ( len > 0 ){
		width = (len > 512) ? 512 : len;
		if ( (ret = (int)read(fd, buf, (size_)width)) != width ){ printf("read error\n"); exit(-1); }
		for ( int i = 0; i < ret; ++i ){
			putchar((i % 16) ? ' ' : '\n');
			putchar(buf[i]);
		}
		putchar('\n');
		len -= ret;
	}
}
#undef BUFLIMIT

long elf_::interactive_info(){
	printf("-------------------------------------------\n");
	printf("[Ctrl+D]:exit\n");
	printf("1.ehdr\n");
	printf("2.phdr\n");
	printf("3.shdr\n");
	printf("[no]\n");
	return TRUE;
}

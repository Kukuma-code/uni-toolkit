/*********************************************
 * mem: グローバル operator new/new[]/delete/delete[] の差し替え
 *
 * 原典: devel/lib/mem/{new,new_a,delete,delete_a}.c。
 * 「malloc 等を個別コードでばらつかせず統一フォーマット（mnode/mallocator）
 * に集約する」という devel の設計中核。new[] は生ブロックのヘッダに
 * CLASS_ARRAY を立て、mnode_refcnt_decr_or_delete が delete / delete[] を
 * 正しく選べるようにする（原典 new_a.c の mnode_set_newcaflag と同一）。
 *
 * 原典からの現代化:
 *   - 動的例外仕様（throw(std::bad_alloc) / throw()）除去（C++17 で削除済み）。
 *   - sized delete（C++14）を追加定義（unsized へ転送）。
 *   - LONG64/ARM_ELF のシグネチャ分岐は std::size_t に一本化。
 *   - 過整列型（alignof > 16）の aligned-new オーバーロードは差し替えない。
 *     それらは処理系既定（システムヒープ）に流れ、対になる aligned-delete
 *     も処理系既定が呼ばれるため整合する。
 *
 * リンク方針: この TU はリンクした実行ファイルのみ差し替えが効く（opt-in）。
 * ホストの一般コードまで巻き込まないため、明示的にリンクする
 * （CMake の mem_global_new オブジェクトライブラリ）。
 * スレッド安全性は無し（原典どおり単一スレッド前提）。
 *********************************************/
#include <cstddef>
#include "mnode.h"
#include "mallocator.h"

/* size 0 は 1 に丸める（C++ の契約: new T[0] も成功して一意ポインタを
   返す。mallocator.alloc は原典どおり 0 を BAD_ARG_ZERO で拒否するため、
   契約の吸収は差し替え層の責務。実データ検証で elf_phdr_[0]（phdr なしの
   relocatable）が踏んだ） */
void* operator new(std::size_t _size){
	if ( _size == 0 ) _size = 1;
	return (void*)mallocator.alloc((size_)_size);
}

void* operator new[](std::size_t _size){
	if ( _size == 0 ) _size = 1;
	unsigned char* tmp = mallocator.alloc((size_)_size);
	mnode_set_newcaflag(tmp);
	/* mnode_get の決定的判別の前提: cookie プレフィックスになり得る
	   ブロック先頭 16 バイトを 0 化する（mnode.h 冒頭 3) 参照）。
	   コンパイラの要素数書込みは本関数の return 後なので消えない。
	   ペイロード整列幅(>=16)により 16 バイトは常にブロック内。 */
	for (int i = 0; i < 16; ++i) tmp[i] = 0;
	return (void*)tmp;
}

void operator delete(void* _ref) noexcept {
	mallocator.dealloc((unsigned char*)_ref);
}

void operator delete[](void* _ref) noexcept {
	mallocator.dealloc((unsigned char*)_ref);
}

void operator delete(void* _ref, std::size_t) noexcept {
	operator delete(_ref);
}

void operator delete[](void* _ref, std::size_t) noexcept {
	operator delete[](_ref);
}

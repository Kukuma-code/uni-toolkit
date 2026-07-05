# mem: mnode + mallocator（アリーナ・シングルトン）
add_library(mem OBJECT mem/mallocator.cpp)
target_include_directories(mem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# mem_global_new: グローバル new/delete の差し替え（opt-in リンク）。
# リンクした実行ファイルだけが mallocator 経由になる。
add_library(mem_global_new OBJECT mem/operator_new.cpp)
target_include_directories(mem_global_new PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# elf: ELF64 リーダ（welf）とダンパ群。string_（uni_ 基盤）に依存する
add_library(elf OBJECT
	elf/elf_ehdr.cpp
	elf/elf_phdr.cpp
	elf/elf_shdr.cpp
	elf/elf_sym.cpp
	elf/elf_rel.cpp
	elf/elf_dyn.cpp
	elf/welf.cpp)
target_include_directories(elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

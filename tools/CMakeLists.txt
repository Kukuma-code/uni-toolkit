# welf_dump: elf_ リーダの非対話ダンプ CLI（工程6 実データ照合用）
add_executable(welf_dump welf_dump.cpp)
target_link_libraries(welf_dump PRIVATE elf mem mem_global_new)

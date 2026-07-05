# テストは依存ゼロの assert ベース（外部フレームワーク不使用）。
# 実行されないコードを持ち込まない、という devel の設計意図に合わせる。
add_executable(smoke_test smoke_test.cpp)
add_test(NAME smoke_test COMMAND smoke_test)

# 工程1: mem 基盤。global new 差し替え（mem_global_new）込みでリンクし、
# 実 ABI の cookie 判別と delete 経路まで通しで検証する。
add_executable(mem_test mem_test.cpp)
target_link_libraries(mem_test PRIVATE mem mem_global_new)
add_test(NAME mem_test COMMAND mem_test)

# 工程2: 統一プリミティブ層（reference_init / operator_base / astr / exception）
add_executable(primitives_test primitives_test.cpp)
target_link_libraries(primitives_test PRIVATE mem mem_global_new)
add_test(NAME primitives_test COMMAND primitives_test)

# 工程3: uni__ 本体 + string_ スモーク
add_executable(uni_test uni_test.cpp)
target_link_libraries(uni_test PRIVATE mem mem_global_new)
add_test(NAME uni_test COMMAND uni_test)

# 工程4: 派生層一式（qty__ 双子 / lnode / element / ballocator / list 他）
add_executable(derived_test derived_test.cpp)
target_link_libraries(derived_test PRIVATE mem mem_global_new)
add_test(NAME derived_test COMMAND derived_test)

# 工程5: elf クラスタ（welf リーダ。合成 ELF64 を自前生成して検証）
add_executable(welf_test welf_test.cpp)
target_link_libraries(welf_test PRIVATE elf mem mem_global_new)
add_test(NAME welf_test COMMAND welf_test)

# 工程6: 実 ELF64 の照合。fixtures/real_elf.c を clang の cross target で
# 本物の ELF64 relocatable にビルドし、welf で解析して検証する
# （AppleClang は -target x86_64-unknown-linux-gnu の -c 出力に対応）
set(REAL_ELF_OBJ ${CMAKE_CURRENT_BINARY_DIR}/real_elf64.o)
add_custom_command(
	OUTPUT ${REAL_ELF_OBJ}
	COMMAND ${CMAKE_CXX_COMPILER} -target x86_64-unknown-linux-gnu -x c -c
	        ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/real_elf.c -o ${REAL_ELF_OBJ}
	DEPENDS fixtures/real_elf.c
	COMMENT "Cross-compiling ELF64 fixture (x86_64-unknown-linux-gnu)")
add_custom_target(real_elf_fixture DEPENDS ${REAL_ELF_OBJ})
add_executable(elf64_real_test elf64_real_test.cpp)
add_dependencies(elf64_real_test real_elf_fixture)
target_link_libraries(elf64_real_test PRIVATE elf mem mem_global_new)
add_test(NAME elf64_real_test COMMAND elf64_real_test)

# ---- 継承テスト環境 + シンボル検証（template = 仕様書、実オブジェクト非常駐） ----

# 目的2: 生成物が意図どおり動くか（汎用継承フィクスチャの実行時 assert）
add_executable(inherit_test inherit/inherit_test.cpp)
target_include_directories(inherit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME inherit_test COMMAND inherit_test)

# 目的1/3/4: シンボル表検査（symcheck）。被験体 probe を .o に落として解析する。
# probe は「使う想定」メソッドだけを odr-use する最小 TU（main 無し）。
set(PROBE_INHERIT_O ${CMAKE_CURRENT_BINARY_DIR}/probe_inherit.o)
set(PROBE_UNI_O     ${CMAKE_CURRENT_BINARY_DIR}/probe_uni.o)
add_custom_command(
	OUTPUT ${PROBE_INHERIT_O}
	COMMAND ${CMAKE_CXX_COMPILER} -std=c++17 -I ${CMAKE_CURRENT_SOURCE_DIR}
	        -c ${CMAKE_CURRENT_SOURCE_DIR}/symcheck/probe_inherit.cpp -o ${PROBE_INHERIT_O}
	DEPENDS symcheck/probe_inherit.cpp inherit/inherit_fixture.h
	COMMENT "Compiling symcheck probe (inherit fixture)")
add_custom_command(
	OUTPUT ${PROBE_UNI_O}
	COMMAND ${CMAKE_CXX_COMPILER} -std=c++17 -I ${CMAKE_SOURCE_DIR}/src
	        -c ${CMAKE_CURRENT_SOURCE_DIR}/symcheck/probe_uni.cpp -o ${PROBE_UNI_O}
	DEPENDS symcheck/probe_uni.cpp
	COMMENT "Compiling symcheck probe (uni__ production)")
add_custom_target(symcheck_probes DEPENDS ${PROBE_INHERIT_O} ${PROBE_UNI_O})

find_program(PYTHON3 NAMES python3 python)
if(PYTHON3)
	add_test(NAME symcheck COMMAND ${PYTHON3}
		${CMAKE_CURRENT_SOURCE_DIR}/symcheck/symcheck.py
		--spec ${CMAKE_CURRENT_SOURCE_DIR}/symcheck/spec.json
		--object inherit=${PROBE_INHERIT_O}
		--object uni=${PROBE_UNI_O}
		--src-root ${CMAKE_SOURCE_DIR}/src)
	# symcheck は probe .o に依存する。CTest は custom target を直接は待たないため、
	# fixture 機構で probe ビルドを前段テストとして直列化する。
	add_test(NAME symcheck_build
		COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target symcheck_probes)
	set_tests_properties(symcheck_build PROPERTIES FIXTURES_SETUP SYMCHECK_PROBES)
	set_tests_properties(symcheck PROPERTIES FIXTURES_REQUIRED SYMCHECK_PROBES)
else()
	message(WARNING "python3 not found: symcheck test skipped")
endif()

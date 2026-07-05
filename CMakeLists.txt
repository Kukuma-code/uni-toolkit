cmake_minimum_required(VERSION 3.21)
project(uni_toolkit CXX)

# C++17: 動的例外仕様が削除された規格。原典（2004頃）の modernize 基準。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# clangd LSP 用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)

/* 工程6 検証用フィクスチャ: clang -target x86_64-unknown-linux-gnu -c で
   本物の ELF64 relocatable にクロスコンパイルされる（tests/CMakeLists.txt）。
   welf 側の期待値（シンボル名・種別・サイズ）はこの内容に対応する。 */
int add(int a, int b){ return a + b; }
int global_var = 42;

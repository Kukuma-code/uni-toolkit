#!/bin/zsh -f
# C++ のテスト一式（ctest）を 1 本のテストとして回す入口。合否は終了コードだけ
# （0 = 全部通った / 非 0 = configure・build・ctest のどれかが落ちた）。
#
# test_*.zsh という名前なので、ファイル名で探す外側のテストランナーがそのまま拾う。
# C++ 用の別ランナーを持たずに、時間切れの扱いや「修正前は落ちる」の検算を共有するため。
#
# ビルド先は人の build/ と分ける（既定 build/wtest。TEST_CTEST_BUILD で上書き）。
# 増分ビルドなので 2 回目以降は速いが、そのぶん依存の正しさに頼っている — ヘッダを
# 変えて作り直されない規則があると古い成果物を検査する（test_symcheck_probe_deps.py）。

emulate -L zsh
setopt no_unset pipefail

typeset -r ROOT=${0:A:h:h}
typeset -r BUILD=${TEST_CTEST_BUILD:-$ROOT/build/wtest}

# zsh -f は PATH を組み直さないので、呼び出し側の PATH に無ければ Homebrew の既定を当たる
cmake_bin=$(command -v cmake 2>/dev/null) || cmake_bin=/opt/homebrew/bin/cmake
ctest_bin=$(command -v ctest 2>/dev/null) || ctest_bin=/opt/homebrew/bin/ctest
[[ -x $cmake_bin && -x $ctest_bin ]] \
  || { print -u2 "test_ctest: cmake / ctest が見つからない"; exit 1 }

# 成功時の出力は捨て、失敗したときだけ末尾を出す（何が落ちたかを wtest -v で読めるように）
step=configure
out=$("$cmake_bin" -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug 2>&1) && {
  step=build
  out=$("$cmake_bin" --build "$BUILD" -j 4 2>&1)
} || { print -r -- "$out" | tail -n 40 >&2; print -u2 "test_ctest: $step に失敗"; exit 1 }
exec "$ctest_bin" --test-dir "$BUILD" --output-on-failure

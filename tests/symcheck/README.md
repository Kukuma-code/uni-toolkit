# symcheck - template オブジェクト生成の検証ハーネス

template class は「仕様書」であり、実オブジェクトは常駐しない。使ったメソッド
だけが implicit instantiation でオブジェクト化される。この性質は普通の実行時
テストでは見えないので、**コンパイル済みオブジェクトのシンボル表**を検査する。

検証方針は姉妹プロジェクト cpp_rewriter を踏襲する:

- **(A) golden スナップショット回帰**: 各被験体の defined シンボル集合を
  `golden/symbols.<tag>.json` に固定。増減したら FAIL（`--update` で再生成）。
  `<tag>` は CTest が渡す `<OS>-<CPU>-<コンパイラ ID>`（例 `Darwin-arm64-AppleClang`）。
  実体化と demangle の綴りはツールチェインで変わるので、**別ツールチェインの golden と
  比べない・上書きしない**。初めてのツールチェインでは golden が新規作成され
  （= その回は回帰を見ていない）、作られたファイルをコミットして以後の基準にする。
  libc++ の abi タグ（`[abi:nqe220106]` 等）は比較から落とす（版の綴りで、実体化の増減ではない）。
- **(B) 不変条件**: present / absent / 依存面 / 危険関数 / 境界検査の常駐。

「外部正解が無いので、現状より悪化したときだけ FAIL」。

## 4 つの目的との対応

| 目的 | 内容                               | 実現                                                                           |
| ---- | ---------------------------------- | ------------------------------------------------------------------------------ |
| 1    | 意図どおり object が生成されるか   | `present`: 使ったメソッドが defined シンボルに在る                             |
| 2    | 生成物が意図どおり動くか           | **本ツール対象外**。`inherit_test` / `uni_test` 等の実行時 assert が担当       |
| 3    | 意図しない機能が追加されていないか | `absent`: 使っていないメソッドが実体化されていない + golden 回帰               |
| 4    | セキュリティ                       | 危険 libc 不参照 / 依存面（undefined 許可リスト）/ 境界検査の常駐 / ソース走査 |

## 構成

```
tests/inherit/
  inherit_fixture.h    汎用 template 継承フィクスチャ（ドメイン非依存）
                       単一基底 holder_/counter_、多重継承 cell_、CRTP node_
  inherit_test.cpp     目的2: 全メソッドを呼んで挙動を assert（ctest: inherit_test）
tests/symcheck/
  probe_inherit.cpp    被験体: フィクスチャの「使う想定」だけを odr-use（main 無し）
  probe_uni.cpp        被験体: 本番 uni__<long,long*> の配列実体化・使う想定だけ
  symcheck.py          ハーネス本体（標準ライブラリ + nm/c++filt のみ）
  spec.json            期待値（present/absent/allowed_undefined/banned/require_together）
  golden/symbols.<tag>.json  defined シンボル集合のスナップショット（ツールチェイン別・コミット対象）
tests/test_symcheck_portable.py  symcheck 自体の不変条件（Mach-O/ELF で危険関数検出が空にならない・
                                 abi タグで golden が落ちない。ctest: symcheck_portable）
```

被験体 probe は「使う想定のメソッドだけ」を呼ぶ最小 TU。どれを使うかを probe が
決め、symcheck が used=実体化 / unused=非実体化 をシンボル表で突き合わせる。

## 実行

CTest 統合済み（`symcheck_build` が probe を .o に落とし、`symcheck` が検査）:

```
cmake --build build && ctest --test-dir build -R "inherit_test|symcheck"
```

手動実行・golden 更新（フィクスチャや被験体を変えたとき）:

```
cmake --build build --target symcheck_probes
python3 tests/symcheck/symcheck.py \
  --spec tests/symcheck/spec.json \
  --object inherit=build/tests/probe_inherit.o \
  --object uni=build/tests/probe_uni.o \
  --src-root src --golden-tag Darwin-arm64-AppleClang --update
```

`--golden-tag` を省くと `golden/symbols.json`（札なし）を読み書きする。CTest は常に札付き。

## spec.json の書き方

`objects.<name>` ごと（`--object <name>=<path>` と対応）:

- `present` / `absent`: demangle 名への**空白無視・部分一致**パターン。
- `allowed_undefined`: undefined シンボルの許可リスト（raw/dem 双方に部分一致）。
  ここに無い外部依存が出たら「攻撃面の拡大」として FAIL。
- `banned_undefined`: 明示的な危険関数の denylist（belt-and-suspenders）。
  **Mach-O の綴り（`_strcpy`）で書く。** symcheck は ELF の raw 名に `_` を前置して
  同じ綴りに揃えてから照合する（揃えないと Linux では検査が黙って空になる）。
- `require_together`: `[A, B, C]` = A が在れば B と C も無ければ FAIL。
  境界検査の常駐（`operator[]` を使うなら `check_qty` / `bad_range` が在る）に使う。

`source_scan.roots`: 危険関数呼び出し（`gets(` / `system(` / `strcpy(` /
`sprintf(` / `scanf(` / `exec*(` 等）をソースから走査。行コメントは簡易除外。

## たたき台の限界・次の一手（未実装）

- present/absent は現状 `long` 実体化のみ。`char`/`string_` や list/element の
  被験体を増やすと網羅性が上がる。
- 依存面チェックは probe 単体の undefined を見る。リンク後実行ファイルに対する
  「実際に到達するシンボル」解析（`--gc-sections` 後の残存）は未対応。
- W^X / スタック保護 / RELRO 等のバイナリ・ハードニング検査は macho/.o では
  限定的なため未実装（実 ELF 実行ファイル段で追加余地）。
- 危険 libc の denylist は代表例のみ。CWE カテゴリ（フォーマット文字列・整数
  オーバーフロー・TOCTOU）へ広げる余地。

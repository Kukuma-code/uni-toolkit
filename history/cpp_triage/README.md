# cpp_triage

古いコードを改善し、使えるレベルにする。

## Requirements

## Project Structure

- `inbox/` … 未処理の投入物（トリアージ対象）。処理後に元を削除する。
- `reviewed/` … salvage 済みコードのみ保持（知識ドキュメントは Obsidian へ）。
  - `uniform.cpp` / `pcg32.hpp` … PCG32 乱数コア。
- `samples/` … 元資料（アーカイブ・書籍等）の backup 置き場。`game_samplesources/`（+ Swift サンプル）の triage は完了（下記）。
- devarm / devel / armpp 由来の triage は完了し、能動開発は cpp_devarm・cpp_devel へ分離済み。

## Design

- トリアージ: inbox → コードは reviewed/、知識ドキュメントは Obsidian(`config/taxonomy.yaml` でカテゴリ解決)に単一配置。
- salvage したコードが独立プロジェクトに育った場合は履歴保持で別 repo へ卒業する（例: devarm→cpp_devarm、devel→cpp_devel）。

## Notes

- 日本語ソースは EUC-JP のことがある（`iconv -f EUC-JP -t UTF-8`）。zip 内の Shift-JIS ファイル名は macOS の `unzip` が書けず失敗することがある（`ditto` や必要ファイルのみ抽出で回避）。
- `game_samplesources`（他人のゲームソース）の triage 完了: クリーンルームで仕様/構造/実装技法を抽出し Obsidian `computing/` に6ノート化、原ソースは削除（原本 zip は `samples/` に backup）。2D クローンの能動開発は cpp_triage 外の新規プロジェクト **`../tile_rpg`** へ卒業（devarm→cpp_devarm と同じパターン）。

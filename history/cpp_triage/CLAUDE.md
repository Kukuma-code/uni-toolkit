# cpp_triage - Project Context for Claude Code

## Environment

- Machine: Mac mini M4, 16GB RAM, 256GB SSD
- Shell: zsh
- Volumes:
  - `/Volumes/workspace/` -- working directory
  - `/Volumes/data/` -- data storage
  - `/Volumes/archive/` -- long-term storage (USB 3.0 HDD)
  - `/Volumes/LocalAI/` -- LocalAI related
- Python venv: `/Volumes/workspace/.venvs/cpp_triage` (direnv / layout_python_auto)
- Editor: Emacs (primary), VSCode, Xcode
- Languages: zsh, Python, Swift, C++, Objective-C

## Naming Conventions

- Use `_` instead of `-` in filenames and identifiers
- Shell scripts: zsh only
- Prompts to LLM: English

## Project

古いコードを改善し、使えるレベルにする。

## Tools

## Project Structure

- `inbox/` … 未処理の投入物（トリアージ対象）。処理後に元を削除する。
- `reviewed/` … salvage 済みコードのみ保持（知識ドキュメントは Obsidian へ）。
  - `uniform.cpp` / `pcg32.hpp` … PCG32 乱数コア。
- `samples/` … 元資料（アーカイブ・書籍等）の backup 置き場。`game_samplesources/`（+ Swift サンプル）の triage は完了（下記）。
- devarm / devel / armpp 由来の triage は完了し、能動開発は cpp_devarm・cpp_devel へ分離済み。

## Key Design Decisions

- トリアージ: inbox → コードは reviewed/、知識ドキュメントは Obsidian(`config/taxonomy.yaml` でカテゴリ解決)に単一配置。
- salvage したコードが独立プロジェクトに育った場合は履歴保持で別 repo へ卒業する（例: devarm→cpp_devarm、devel→cpp_devel）。

## Notes

- 日本語ソースは EUC-JP のことがある（`iconv -f EUC-JP -t UTF-8`）。zip 内の Shift-JIS ファイル名は macOS の `unzip` が書けず失敗することがある（`ditto` や必要ファイルのみ抽出で回避）。
- `game_samplesources`（他人のゲームソース）の triage 完了: クリーンルームで仕様/構造/実装技法を抽出し Obsidian `computing/` に6ノート化、原ソースは削除（原本 zip は `samples/` に backup）。2D クローンの能動開発は cpp_triage 外の新規プロジェクト **`../tile_rpg`** へ卒業（devarm→cpp_devarm と同じパターン）。

<!-- pull: この CLAUDE.md は overview.md からの全生成物（手動編集は次回の pull で
     CLAUDE.local.md へ退避される）。
     内容の追記                    → overview.md を編集して pull --update
     手動ドキュメント・ローカルメモ → CLAUDE.local.md（git 追跡が既定）
     セッション状態（SESSION_NO）  → .claude/session.local.md（gitignored）
     どちらも下の import で読み込まれる -->

@CLAUDE.local.md

@.claude/session.local.md

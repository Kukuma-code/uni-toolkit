<!-- プロジェクト名を記述してください
PROJECT_NAME:cpp_triage
-->

# overview

<!-- このファイルが pull command の入力源（Single Source of Truth）です。
     先頭の <!-- PROJECT_NAME:xxx --> を編集してプロジェクト名を設定してください。

     編集後に `pull overview.md` を実行すると CLAUDE.md / .envrc が生成されます。 -->
     過去に書いた古いコードを分別し、使えるコードを再編成する。

## description

<!-- プロジェクトの概要を書いてください。CLAUDE.md の "Project" セクションになります -->

古いコードを改善し、使えるレベルにする。

## tools

<!-- 使用するツール・サービスを列挙してください。
     以下のキーワードを含むと .envrc に対応ブロックが自動挿入されます：
       postgresql      → PG 接続情報 + データ/ログパス
       ollama          → OLLAMA_MODEL 変数
       comfyui         → COMFYUI_DIR / COMFYUI_API / COMFYUI_PORT
       zsh_completions → DIRENV_ZSH_COMPLETIONS（scripts/completions/ を fpath 管理）
     以下のキーワードを含むと CLAUDE.md に設計ノートが追記されます：
       launchd         → /Volumes/ 制限・ローカルランチャーパターンの設計ルール
       ollama または comfyui → ai_switch による GPU 排他制御ノート
     常に生成されるもの：
       venv        → layout_python_auto + PATH_add scripts
       archive     → PROJECT_ARCHIVE_ROOT / PROJECT_LOG_DIR
       log         → log_format.md（log_script の SSOT をパス参照）

     記入例:
       - postgresql
       - ollama -->

## structure

<!-- ディレクトリ構成や主要ファイルを書いてください。CLAUDE.md の "Project Structure" になります -->

- `inbox/` … 未処理の投入物（トリアージ対象）。処理後に元を削除する。
- `reviewed/` … salvage 済みコードのみ保持（知識ドキュメントは Obsidian へ）。
  - `uniform.cpp` / `pcg32.hpp` … PCG32 乱数コア。
- `samples/` … 元資料（アーカイブ・書籍等）の backup 置き場。`game_samplesources/`（+ Swift サンプル）の triage は完了（下記）。
- devarm / devel / armpp 由来の triage は完了し、能動開発は cpp_devarm・cpp_devel へ分離済み。

## design

<!-- 設計上の決定・制約を書いてください。CLAUDE.md の "Key Design Decisions" になります -->

- トリアージ: inbox → コードは reviewed/、知識ドキュメントは Obsidian(`config/taxonomy.yaml` でカテゴリ解決)に単一配置。
- salvage したコードが独立プロジェクトに育った場合は履歴保持で別 repo へ卒業する（例: devarm→cpp_devarm、devel→cpp_devel）。

## env

<!-- .envrc に追加したいカスタム環境変数を書いてください。
     postgresql / ollama ブロックは ## tools で制御します。
     ここに書いた内容はそのまま .envrc に挿入されます。

     記入例:
       export TARGET_DB="my_db"
       export MODEL_CACHE_DIR=/Volumes/workspace/models -->

## notes

<!-- その他のメモ・注意事項を書いてください。CLAUDE.md の "Notes" になります -->

- 日本語ソースは EUC-JP のことがある（`iconv -f EUC-JP -t UTF-8`）。zip 内の Shift-JIS ファイル名は macOS の `unzip` が書けず失敗することがある（`ditto` や必要ファイルのみ抽出で回避）。
- `game_samplesources`（他人のゲームソース）の triage 完了: クリーンルームで仕様/構造/実装技法を抽出し Obsidian `computing/` に6ノート化、原ソースは削除（原本 zip は `samples/` に backup）。2D クローンの能動開発は cpp_triage 外の新規プロジェクト **`../tile_rpg`** へ卒業（devarm→cpp_devarm と同じパターン）。

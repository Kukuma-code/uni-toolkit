#!/usr/bin/env python3
"""symcheck - template オブジェクト生成の検証ハーネス（cpp_rewriter 流）

template class は「仕様書」であり、実オブジェクトは常駐しない（使ったメソッドだけ
implicit instantiation で実体化される）。この性質は普通の実行時テストでは見えない。
symcheck はコンパイル済みオブジェクトの「シンボル表」を検査し、4 目的を確認する:

  目的1 意図どおりオブジェクトが生成されるか
        -> 使ったメソッドが defined シンボルとして存在する（present）
  目的2 生成物が意図どおり動くか
        -> これは inherit_test / uni_test 等の実行時 assert が担う（本ツール対象外）
  目的3 意図しない機能が追加されていないか
        -> 使っていないメソッドが実体化されていない（absent）
        -> 宣言のみ/未実装の危険機能がオブジェクトに現れない
        -> defined シンボル集合が golden から増減していない（回帰）
  目的4 セキュリティ
        -> 危険 libc（gets/system/strcpy/sprintf/scanf ...）を参照しない
           （object の undefined シンボル + ソース走査の二重チェック）
        -> 依存面（undefined シンボル）が許可リスト内に収まる（想定外の外部依存＝面拡大）
        -> 境界検査の常駐（indexing を使うなら check_qty/bad_range が必ず在る）

検証方針は cpp_rewriter を踏襲:
  (A) golden スナップショット回帰（defined シンボル集合を固定、--update で更新）
  (B) 不変条件（present/absent/依存面/危険関数/境界検査）
「外部正解が無いので、現状より悪化したときだけ FAIL」。

依存: 標準ライブラリのみ + nm / c++filt（macOS/Linux 標準）。pytest 不使用。
"""
import argparse
import json
import os
import re
import subprocess
import sys

WS_RE = re.compile(r"\s+")


def norm(s):
    """空白を畳んで比較を安定させる（demangle の空白差を吸収）。"""
    return WS_RE.sub("", s)


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def demangle(names):
    """raw シンボル名の配列を c++filt で demangle（一括）。失敗時は素通し。"""
    if not names:
        return {}
    try:
        p = run(["c++filt"], input="\n".join(names))
        out = p.stdout.splitlines()
        if len(out) == len(names):
            return dict(zip(names, out))
    except FileNotFoundError:
        pass
    return {n: n for n in names}


DEFINED_TYPES = set("TtWwSsDdBbRrVvGg")


def read_symbols(obj):
    """nm -g の出力を defined / undefined に分類。demangle 済みを付す。

    返り値: (defined:[{raw,dem,type}], undefined:[{raw,dem,type}])
    """
    p = run(["nm", "-g", obj])
    if p.returncode != 0:
        raise RuntimeError(f"nm failed for {obj}: {p.stderr.strip()}")
    defined_raw, undef_raw = [], []
    for line in p.stdout.splitlines():
        toks = line.split()
        if len(toks) == 3:            # <addr> <type> <name>
            _, typ, name = toks
        elif len(toks) == 2:          # <type> <name>  (undefined: 空アドレス)
            typ, name = toks
        else:
            continue
        if typ == "U":
            undef_raw.append(name)
        elif typ in DEFINED_TYPES:
            defined_raw.append((typ, name))
    allnames = [n for _, n in defined_raw] + undef_raw
    dm = demangle(allnames)
    defined = [{"raw": n, "dem": dm.get(n, n), "type": t} for t, n in defined_raw]
    undefined = [{"raw": n, "dem": dm.get(n, n), "type": "U"} for n in undef_raw]
    return defined, undefined


def any_contains(symbols, pattern, key="dem"):
    np = norm(pattern)
    return any(np in norm(s[key]) for s in symbols)


def matches(symbols, pattern, key="dem"):
    np = norm(pattern)
    return [s for s in symbols if np in norm(s[key])]


class Report:
    def __init__(self):
        self.fails = []
        self.notes = []

    def check(self, cond, msg):
        (self.notes if cond else self.fails).append(("PASS" if cond else "FAIL", msg))
        return cond

    def note(self, msg):
        self.notes.append(("INFO", msg))


def check_object(name, obj, spec, rep, golden, update):
    defined, undefined = read_symbols(obj)
    def_dem = sorted({s["dem"] for s in defined})

    # 目的1: present（使ったメソッドが実体化されている）
    for pat in spec.get("present", []):
        rep.check(any_contains(defined, pat),
                  f"[{name}] present: {pat}")

    # 目的3: absent（使っていない機能が実体化されていない）
    for pat in spec.get("absent", []):
        hit = matches(defined, pat)
        rep.check(not hit,
                  f"[{name}] absent: {pat}" + (f"  <-- 混入: {[h['dem'] for h in hit]}" if hit else ""))

    # 目的4a: 依存面（undefined が許可リスト内）
    allow = [norm(a) for a in spec.get("allowed_undefined", [])]
    if allow:
        for s in undefined:
            ok = any(a in norm(s["raw"]) or a in norm(s["dem"]) for a in allow)
            rep.check(ok, f"[{name}] 依存面: 外部シンボル '{s['dem']}'"
                      + ("（許可内）" if ok else "（許可リスト外＝面拡大）"))

    # 目的4b: 危険 libc を参照しない（undefined の denylist）
    for bad in spec.get("banned_undefined", []):
        hit = [s["dem"] for s in undefined if norm(bad) in norm(s["raw"]) or norm(bad) in norm(s["dem"])]
        rep.check(not hit, f"[{name}] 危険関数参照: {bad}" + (f"  <-- {hit}" if hit else ""))

    # 目的4c: 境界検査の常駐（A があるなら B/C も無ければならない）
    for group in spec.get("require_together", []):
        trigger = group[0]
        if any_contains(defined, trigger):
            for need in group[1:]:
                rep.check(any_contains(defined, need),
                          f"[{name}] 境界/安全機構の欠落: '{trigger}' 使用時は '{need}' が必要")

    # 目的3(回帰): golden スナップショット（defined シンボル集合）
    prev = golden.get(name)
    if update or prev is None:
        golden[name] = def_dem
        rep.note(f"[{name}] golden {'更新' if prev is not None else '新規作成'}: {len(def_dem)} defined symbols")
    else:
        added = [s for s in def_dem if s not in prev]
        removed = [s for s in prev if s not in def_dem]
        if added or removed:
            for a in added:
                rep.check(False, f"[{name}] golden 増加（未知の実体化）: {a}")
            for r in removed:
                rep.check(False, f"[{name}] golden 減少（実体化の消失）: {r}")
        else:
            rep.note(f"[{name}] golden 一致: {len(def_dem)} defined symbols")


BANNED_SRC_CALLS = [
    r"\bgets\s*\(", r"\bsystem\s*\(", r"\bpopen\s*\(",
    r"\bstrcpy\s*\(", r"\bstrcat\s*\(", r"\bsprintf\s*\(", r"\bvsprintf\s*\(",
    r"\bscanf\s*\(", r"\bexecl\w*\s*\(", r"\bexecv\w*\s*\(",
]


def source_scan(roots, rep, allow_paths):
    pats = [re.compile(p) for p in BANNED_SRC_CALLS]
    allow = set(allow_paths or [])
    hits = []
    for root in roots:
        for dirpath, _, files in os.walk(root):
            for fn in files:
                if not fn.endswith((".c", ".cpp", ".h", ".hpp")):
                    continue
                path = os.path.join(dirpath, fn)
                rel = os.path.relpath(path)
                if rel in allow:
                    continue
                try:
                    with open(path, encoding="utf-8", errors="replace") as f:
                        for i, line in enumerate(f, 1):
                            code = line.split("//", 1)[0]      # 行コメントは除外（簡易）
                            for p in pats:
                                if p.search(code):
                                    hits.append(f"{rel}:{i}: {line.strip()}")
                except OSError:
                    pass
    rep.check(not hits, "ソース走査: 危険関数呼び出しなし"
              + ("\n    " + "\n    ".join(hits) if hits else ""))


def main():
    ap = argparse.ArgumentParser(description="template オブジェクト生成の検証ハーネス")
    ap.add_argument("--spec", required=True)
    ap.add_argument("--object", action="append", default=[],
                    help="NAME=PATH（複数指定可）。spec の objects と対応させる")
    ap.add_argument("--src-root", action="append", default=[],
                    help="ソース走査ルート（複数指定可）")
    ap.add_argument("--golden", help="golden スナップショット JSON パス（spec からの相対も可）")
    ap.add_argument("--update", action="store_true", help="golden を再生成する")
    args = ap.parse_args()

    with open(args.spec) as f:
        spec = json.load(f)
    spec_dir = os.path.dirname(os.path.abspath(args.spec))

    golden_path = args.golden or spec.get("golden")
    if golden_path and not os.path.isabs(golden_path):
        golden_path = os.path.join(spec_dir, golden_path)
    golden = {}
    if golden_path and os.path.exists(golden_path) and not args.update:
        with open(golden_path) as f:
            golden = json.load(f)

    objects = {}
    for kv in args.object:
        k, _, v = kv.partition("=")
        objects[k] = v

    rep = Report()

    for name, ospec in spec.get("objects", {}).items():
        path = objects.get(name)
        if not path:
            rep.check(False, f"[{name}] object path 未指定（--object {name}=...）")
            continue
        if not os.path.exists(path):
            rep.check(False, f"[{name}] object 不在: {path}")
            continue
        check_object(name, path, ospec, rep, golden, args.update)

    roots = args.src_root or spec.get("source_scan", {}).get("roots", [])
    if roots:
        source_scan(roots, rep, spec.get("source_scan", {}).get("allow_paths", []))

    if golden_path and (args.update or not os.path.exists(golden_path)):
        os.makedirs(os.path.dirname(golden_path), exist_ok=True)
        with open(golden_path, "w") as f:
            json.dump(golden, f, indent=2, ensure_ascii=False, sort_keys=True)
            f.write("\n")

    print("=" * 60)
    print("symcheck report")
    print("=" * 60)
    for status, msg in rep.notes:
        print(f"  [{status}] {msg}")
    for status, msg in rep.fails:
        print(f"  [{status}] {msg}")
    print("-" * 60)
    print(f"PASS={len(rep.notes)}  FAIL={len(rep.fails)}")
    return 1 if rep.fails else 0


if __name__ == "__main__":
    sys.exit(main())

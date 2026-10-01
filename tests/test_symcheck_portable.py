#!/usr/bin/env python3
"""symcheck の検査がツールチェインに依存しないことの不変条件。

不変条件 A: 危険 libc 関数を参照するオブジェクトは、どの形式（Mach-O / ELF）でも
            banned として検出され、参照しないオブジェクトは検出されない。
不変条件 B: golden 回帰は実体化の増減だけで落ち、abi タグの綴りでは落ちない
            （TestGoldenIgnoresAbiTagSpelling）。

背景（2026-10-01）: spec.json の banned_undefined は Mach-O の C シンボル接頭辞付きで
書かれている（"_strcpy"）。ELF では C 関数が接頭辞無し（"strcpy"）で出るため、
Linux では危険関数を参照しても検査が黙って空になっていた。

各形式のオブジェクトは手元のコンパイラで実際に作る（作れない形式・nm が読めない
形式は skip。少なくともホスト既定の形式は必ず走る）。標準ライブラリのみ。
"""
from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
SPEC = os.path.join(HERE, "symcheck", "spec.json")
sys.path.insert(0, os.path.join(HERE, "symcheck"))

import symcheck  # noqa: E402

USES_STRCPY = (
    'extern "C" char *strcpy(char *, const char *);\n'
    "char buf[8];\n"
    "void use(const char *s){ strcpy(buf, s); }\n"
)
CLEAN = "int twice(int a){ return a * 2; }\n"

# 形式名 -> 追加フラグ（None = ホスト既定）
FORMATS = {
    "host": None,
    "elf": ["-target", "x86_64-unknown-linux-gnu"],
    "macho": ["-target", "arm64-apple-macos11"],
}


def compiler() -> str | None:
    return os.environ.get("CXX") or shutil.which("c++") or shutil.which("clang++")


def banned_list() -> list[str]:
    with open(SPEC) as f:
        spec = json.load(f)
    return spec["objects"]["uni"]["banned_undefined"]


class TestBannedDetectionIsFormatIndependent(unittest.TestCase):
    def setUp(self):
        self.cxx = compiler()
        if self.cxx is None:
            self.skipTest("C++ コンパイラが無い")
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)

    def build(self, fmt: str, source: str) -> str:
        flags = FORMATS[fmt]
        if flags is not None and "clang" not in subprocess.run(
                [self.cxx, "--version"], capture_output=True, text=True).stdout.lower():
            self.skipTest(f"{fmt}: -target は clang 専用")
        src = os.path.join(self.tmp.name, f"{fmt}_{abs(hash(source))}.cpp")
        obj = src[:-4] + ".o"
        with open(src, "w") as f:
            f.write(source)
        r = subprocess.run([self.cxx, *(flags or []), "-c", src, "-o", obj],
                           capture_output=True, text=True)
        if r.returncode != 0:
            self.skipTest(f"{fmt}: コンパイルできない: {r.stderr.strip()[:200]}")
        if subprocess.run(["nm", "-g", obj], capture_output=True).returncode != 0:
            self.skipTest(f"{fmt}: この nm では読めない")
        return obj

    def banned_failures(self, obj: str) -> list[str]:
        rep = symcheck.Report()
        symcheck.check_object("t", obj, {"banned_undefined": banned_list()},
                              rep, {}, False)
        return [msg for _, msg in rep.fails if "危険関数参照" in msg]

    def test_strcpy_reference_is_detected(self):
        for fmt in FORMATS:
            with self.subTest(fmt=fmt):
                fails = self.banned_failures(self.build(fmt, USES_STRCPY))
                self.assertTrue(any("strcpy" in m for m in fails),
                                f"{fmt}: strcpy 参照を検出しない: {fails}")

    def test_clean_object_has_no_banned_hit(self):
        for fmt in FORMATS:
            with self.subTest(fmt=fmt):
                self.assertEqual(self.banned_failures(self.build(fmt, CLEAN)), [])


class TestGoldenIgnoresAbiTagSpelling(unittest.TestCase):
    """不変条件: golden 回帰は実体化の増減だけで落ち、標準ライブラリの版の綴り
    （libc++ の [abi:...] タグ）の違いでは落ちない。

    背景（2026-10-01）: Xcode 更新で std::exception::exception[abi:nqe210106]() が
    [abi:nqe220106] になり、コードは無変更のまま golden が「増加 1・減少 1」で落ちた。
    """

    def setUp(self):
        cxx = compiler()
        if cxx is None:
            self.skipTest("C++ コンパイラが無い")
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.obj = os.path.join(self.tmp.name, "probe_uni.o")
        r = subprocess.run(
            [cxx, "-std=c++17", "-I", os.path.join(os.path.dirname(HERE), "src"),
             "-c", os.path.join(HERE, "symcheck", "probe_uni.cpp"), "-o", self.obj],
            capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stderr)
        defined, _ = symcheck.read_symbols(self.obj)
        self.dem = sorted({s["dem"] for s in defined})

    def golden_failures(self, prev: list[str]) -> list[str]:
        rep = symcheck.Report()
        symcheck.check_object("t", self.obj, {}, rep, {"t": prev}, False)
        return [msg for _, msg in rep.fails if "golden" in msg]

    def test_other_abi_tag_version_still_matches(self):
        tagged = [d for d in self.dem if "[abi:" in d]
        if not tagged:
            self.skipTest("このツールチェインは abi タグを出さない")
        prev = [symcheck.ABI_TAG_RE.sub("[abi:zzz000000]", d) for d in self.dem]
        self.assertEqual(self.golden_failures(prev), [])

    def test_real_instantiation_change_is_still_caught(self):
        self.assertTrue(self.dem, "defined シンボルが無い")
        self.assertTrue(self.golden_failures(self.dem[1:]), "1 件増えても落ちない")
        self.assertTrue(self.golden_failures(self.dem + ["fake_extra()"]),
                        "1 件減っても落ちない")


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""symcheck の被験体 probe の不変条件。

不変条件: probe の .o は、それが include するヘッダが変わったら作り直される。
          作り直されないと、symcheck は古い .o を検査して黙って PASS する。

背景（2026-10-03）: probe の add_custom_command は DEPENDS に probe の .cpp しか
持たず、src/mem/mnode.h を変えても probe_uni.o が古いまま検査された（手元の
増分ビルドだけ。CI は毎回新規ビルドなので当たらない）。

ソース木を一時ディレクトリへ写して configure し、probe だけをビルドする
（元の木とビルドには触らない）。標準ライブラリのみ。
"""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
import time
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
COPY = ("CMakeLists.txt", "src", "tests", "tools")
IGNORE = shutil.ignore_patterns("__pycache__", ".DS_Store")


class TestProbeRebuildsOnHeaderChange(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if shutil.which("cmake") is None:
            raise unittest.SkipTest("cmake が無い")
        cls.tmp = tempfile.TemporaryDirectory()
        cls.src = os.path.join(cls.tmp.name, "src_tree")
        cls.bld = os.path.join(cls.tmp.name, "build")
        os.makedirs(cls.src)
        for name in COPY:
            p = os.path.join(ROOT, name)
            if os.path.isdir(p):
                shutil.copytree(p, os.path.join(cls.src, name), ignore=IGNORE)
            else:
                shutil.copy2(p, os.path.join(cls.src, name))
        cfg = ["cmake", "-S", cls.src, "-B", cls.bld]
        if os.environ.get("CXX"):
            cfg.append(f"-DCMAKE_CXX_COMPILER={os.environ['CXX']}")
        r = subprocess.run(cfg, capture_output=True, text=True)
        if r.returncode != 0:
            cls.tmp.cleanup()
            raise RuntimeError(f"configure に失敗: {r.stderr[-2000:]}")

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def build_probes(self):
        r = subprocess.run(["cmake", "--build", self.bld, "--target", "symcheck_probes"],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout[-2000:] + r.stderr[-2000:])

    def obj_mtime(self, name: str) -> int:
        return os.stat(os.path.join(self.bld, "tests", name)).st_mtime_ns

    def bump(self, rel: str):
        """ヘッダの mtime を確実に新しくする（中身は変えない）。"""
        p = os.path.join(self.src, rel)
        t = time.time() + 10
        os.utime(p, (t, t))

    def check(self, obj: str, header: str):
        self.build_probes()
        before = self.obj_mtime(obj)
        self.build_probes()
        # 対照: 何も変えなければ作り直さない（下の比較が常に真になる測り方でないこと）
        self.assertEqual(self.obj_mtime(obj), before, f"{obj}: 無変更でも作り直した")
        self.bump(header)
        self.build_probes()
        self.assertNotEqual(self.obj_mtime(obj), before,
                            f"{obj}: {header} を変えても作り直さない")

    def test_uni_probe_follows_src_headers(self):
        self.check("probe_uni.o", os.path.join("src", "mem", "mnode.h"))

    def test_inherit_probe_follows_fixture_header(self):
        self.check("probe_inherit.o", os.path.join("tests", "inherit", "inherit_fixture.h"))


if __name__ == "__main__":
    unittest.main()

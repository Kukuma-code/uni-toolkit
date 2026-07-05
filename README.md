# uni-toolkit

A host-side C++17 toolkit rebuilt from the author's own early-2000s C++
codebase. The original was a CVS-era tree that generated its template layer
with Perl code generators; this rebuild replaces the code generation with
native C++17 templates while preserving the original architecture and its
invariants.

Source comments are written in Japanese. They document, for each module, the
original design intent and every deviation the rebuild makes from the
original (modernizations, bug fixes, ABI adaptations).

## Components

- `src/mem/` — region allocator with an intrusive node header (`mnode_`)
  carrying a reference count, plus a global `operator new` replacement.
  The header-locating logic replaces the original's probing heuristics
  (undefined behavior on modern ABIs) with deterministic detection based on
  `constexpr` array-cookie size computation per type (AArch64 two-word and
  x86-64 one-word `new[]` cookies are both handled).
- `src/uni/` — the `uni_` unified-primitive layer: a single template
  hierarchy (`reference_init` / `operator_base` / bounds checking) under
  which strings, scalars, lists and allocator wrappers share one storage
  and reference-counting protocol.
- `src/elf/` — an ELF64 reader (`welf`) and section/segment/symbol dumpers.
  Fixed-width `<cstdint>` layout with the ELF specification constants
  vendored; no dependency on a system `<elf.h>`.
- `src/a/` — minimal string/print output layer used by the dumpers.
- `tools/welf_dump` — non-interactive CLI for dumping real ELF64 binaries.

## Design notes

- **Templates are the specification.** The derived layer is ported in full;
  what a given program actually instantiates is decided by the compiler
  (implicit instantiation), not by trimming the source.
- **Only-used instantiation is verified, not assumed.** `tests/symcheck/`
  checks the symbol tables of compiled objects against golden snapshots:
  used features must be present, unused features must not leak in, and
  bounds-checking code must be resident whenever indexing is used.
- Exceptions are the original's own `bad_ctor` / `bad_range` types,
  kept as-is under C++17 semantics.

## Build and test

Requires CMake >= 3.21 and a C++17 compiler (developed with clang on
AArch64 macOS; the test suite also passes under x86-64).

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

The test suite is dependency-free (assert-based). The full acceptance bar
used during development is: all tests green, ASan+UBSan clean, and zero
warnings under `-Wall -Wextra`.

## License

MIT License — Copyright (c) 2004-2026 Yuichi Makihara (牧原雄一).
The range starts at the original codebase's creation; this repository
is a rebuild of the author's own prior work. See [LICENSE](LICENSE).

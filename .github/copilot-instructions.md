# Jaymod — Copilot Coding Instructions

## Project overview

**Jaymod** is a Wolfenstein: Enemy Territory game modification (mod) written in C++. It is built using a custom GNU Make framework and produces shared libraries (`.so` / `.dll` / `.dylib`) loaded by the game engine at runtime.

The project shares its build framework with **xmod** — changes to `make/`, `project/`, and portability patterns follow the same conventions.

---

## Repository layout

```
src/
  base/        # Shared utilities (strings, containers, config, text)
  bgame/       # Shared game logic (both client & server)
  cgame/       # Client-side game code
  game/        # Server-side game code (qagame)
  ui/          # UI module code
  lua/         # Embedded Lua interpreter
  sqlite3/     # Embedded SQLite
make/
  platform/    # Per-platform compiler settings (one file per platform)
  variant/     # Per-variant build flags (release / debug)
  main.mk      # Main build logic
  fn.mk        # Helper functions
project/
  info.py      # Build metadata generator (Python 3)
  info.db      # Per-platform/variant metadata
.github/
  workflows/   # GitHub Actions CI definitions
```

---

## Supported platforms

| Platform key | Arch | OS | Toolchain |
|---|---|---|---|
| `linux` | i386 | Linux | `g++` (32-bit, `gcc-multilib`) |
| `linux64` | x86_64 | Linux | `g++` |
| `linux-aarch64` | aarch64 | Linux | `aarch64-linux-gnu-g++` (cross) |
| `mingw` | i386 | Windows | `i686-w64-mingw32-g++` |
| `mingw64` | x86_64 | Windows | `x86_64-w64-mingw32-g++` |
| `osx64` | x86_64 | macOS | `g++` (Xcode) |
| `osx-arm64` | arm64 | macOS | `g++` (Xcode, Apple Silicon) |

To build: `PLATFORM=<key> make release`

---

## Build system

- **No CMake / autoconf** — pure GNU Make.
- Entry point: `GNUmakefile` → includes `make/main.mk`.
- Platform file sets compiler, flags, suffixes. Variant file sets optimisation/debug flags.
- `project/info.py` must be executable (`chmod +x project/info.py`); it generates version/revision headers.

### Adding a new platform

1. Create `make/platform/<name>` — either from scratch or with `include make/platform/<base>` to inherit settings and override specific variables.
2. Create `make/variant/<name>-release` and `make/variant/<name>-debug` (usually identical to an existing platform's variants).
3. Add a `::<name>` section in `project/info.db` with `platformName = ...`.
4. Add `JAYMOD_<NAME>` to the `#if` chain in `src/base/config.h`.
5. If the platform is 64-bit, include it in the guards in `src/base/text/InlineText.h` and `src/base/text/InlineText.cpp`.

---

## 64-bit portability rules

Jaymod's original code was written for 32-bit only. When adding 64-bit targets, follow these rules:

1. **No pointer-to-int casts** — use `intptr_t` or `uintptr_t` (from `<stdint.h>`).
2. **Field offsets** — use `offsetof(Type, field)` instead of `(int)&((Type*)0)->field` (the null-pointer cast truncates on 64-bit).
3. **`size_t` for sizes** — use `size_t` (not `int` or `uint32`) when dealing with memory sizes or pointer arithmetic.
4. **Pointer comparisons** — `ptr == '\0'` is always wrong; use `*ptr == '\0'` or `ptr == nullptr`.
5. **`vmMain` return type** — must be `intptr_t`, not `int`.
6. **`std::byte` ambiguity** — use `-std=gnu++14` on GCC to suppress the ambiguity with C99 hex floats in `etpro_mdx_lut.h`.

---

## Code style

- C++14 (`-std=gnu++14`), no exceptions (`-fno-exceptions`), no RTTI (`-fno-rtti`).
- Existing code uses tabs for indentation — match the surrounding style.
- Class names: `CamelCase`. Functions: `camelCase`. Constants/macros: `UPPER_SNAKE`.
- Platform-specific code goes under `#if defined( JAYMOD_<PLATFORM> )` guards.

---

## CI / GitHub Actions

| Workflow | Trigger | Purpose |
|---|---|---|
| `build-quick-test.yml` | push/PR | linux64 qagame smoke test |
| `build-quick-mixed.yml` | push/PR | linux64 qagame + mingw32/64 cgame+ui (faster feedback) |
| `build-multiplatform.yml` | push/PR/dispatch | All platforms |
| `build-release.yml` | manual dispatch | All platforms + optional GitHub release |
| `build-docs.yml` | manual dispatch | DocBook documentation |

- Windows is **cross-compiled** on `ubuntu-latest` via MinGW.
- macOS builds run on `macos-latest`.
- ARM64 Linux is **cross-compiled** on `ubuntu-latest` via `gcc-aarch64-linux-gnu`.

---

## Common pitfalls

- Do not call `BG_WeaponForMOD` twice for the same value (performance).
- `CG_PUMPEVENTLOOP` is not implemented engine-side — do not use it.
- `SendServerCommand` argument ordering matters — check ET protocol docs.
- SVN metadata is optional — `project/info.py` returns empty defaults if SVN is not available (non-fatal).

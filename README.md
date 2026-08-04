# Jaymod

This is the source for Jaymod, an Enemy Territory mod.

## Building

The mod uses a GNU make build system that cross-compiles for Linux (32-bit and 64-bit) and Windows (32-bit and 64-bit via MinGW). All builds run on Ubuntu — no Windows runner is needed.

### Prerequisites

**All platforms:**
```
build-essential python3 zip m4
```

**Linux 32-bit only:**
```
gcc-multilib g++-multilib
```

**Windows cross-compile only:**
```
mingw-w64
```

### Build Commands

```bash
# Make project script executable (required once)
chmod +x project/info.py

# Linux 64-bit (recommended)
PLATFORM=linux64 make release

# Linux 32-bit
PLATFORM=linux make release

# Windows 32-bit (cross-compiled with MinGW)
PLATFORM=mingw make release

# Windows 64-bit (cross-compiled with MinGW)
PLATFORM=mingw64 make release
```

### Output Artifacts

| Platform  | qagame | cgame | ui |
|-----------|--------|-------|----|
| linux64   | `build.linux64-release/game/qagame.mp.x86_64.so` | `build.linux64-release/cgame/cgame.mp.x86_64.so` | `build.linux64-release/ui/ui.mp.x86_64.so` |
| linux32   | `build.linux-release/game/qagame.mp.i386.so` | `build.linux-release/cgame/cgame.mp.i386.so` | `build.linux-release/ui/ui.mp.i386.so` |
| win32     | `build.mingw-release/game/qagame_mp_x86.dll` | `build.mingw-release/cgame/cgame_mp_x86.dll` | `build.mingw-release/ui/ui_mp_x86.dll` |
| win64     | `build.mingw64-release/game/qagame_mp_x64.dll` | `build.mingw64-release/cgame/cgame_mp_x64.dll` | `build.mingw64-release/ui/ui_mp_x64.dll` |

## GitHub Actions Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| `build-quick-test.yml` | Push/PR to `master`, `feature/**`, `fix/**`, `copilot/**` | Builds qagame for linux64 as a smoke test |
| `build-multiplatform.yml` | Push/PR to `master` + dispatch | Builds all platforms (linux64, linux32, win32, win64) |
| `build-release.yml` | Manual dispatch | Builds all platforms + pak data; optionally creates a GitHub release |
| `build-docs.yml` | Manual dispatch | Builds HTML/PDF documentation from DocBook sources |

To trigger a release: go to **Actions → Release Build → Run workflow**, check *Create a GitHub release*.

## Build System Notes

More details about the build system can be found in [notes/BuildSystem.txt](notes/BuildSystem.txt).

## License

This source is bound to the original terms from **id Software**. On top of that, this source is released under the Apache 2.0 license. Feel free to use this codebase as you please, as long as both licenses are bundled and proper credit is given.


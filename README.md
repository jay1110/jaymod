# Jaymod

This is the source for the last release of Jaymod, which is version 2.0.0.

# Features

## Lua Integration

Lua 5.4.7 is fully integrated into the mod for all supported platforms:

- **Linux 32-bit** (i386)
- **Linux 64-bit** (x86_64)
- **Windows 32-bit** (x86)
- **Windows 64-bit** (x64)

Lua is compiled as a static library and linked directly into the game modules, so no external Lua installation is required.

# Compiling

The mod has a very powerful make system that can cross-compile the mod for Linux, Windows (via mingw) and OSX. There are also specialty binaries for specific CPU
platforms for each of these platforms.

## Build Commands

Build for specific platforms using:

```bash
# Linux 64-bit
PLATFORM=linux64 make release

# Linux 32-bit (requires gcc-multilib/g++-multilib)
PLATFORM=linux make release

# Windows 32-bit (cross-compile via MinGW)
PLATFORM=mingw make release

# Windows 64-bit (cross-compile via MinGW)
PLATFORM=mingw64 make release
```

Information about the build system can be found in the [build system notes](notes/BuildSystem.txt).

Some of the compilation and system libraries required to make a complete build have been lost to time. If anyone would like to minimalize the build system and provide
compilation instructions, it would be much appreciated.

# License

This source is bound to the original terms from **id Software**. On top of that, I am releasing this source under the Apache 2.0 license. Feel free to use this codebase
as you please, as long as both licenses are bundled and proper credit is given.


# 3GSEmu

A from-scratch iPhone 3GS emulator prototype.

## Build

### Linux / macOS

```bash
git clone https://github.com/maplemilesalt-byte/3GSEmu.git
cd 3GSEmu
cmake -S . -B build
cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
./build/3gs-emu
```

If your system does not have `nproc`, use a normal build instead:

```bash
cmake --build build
```

### Windows

With Visual Studio's C++ tools and CMake available:

```powershell
git clone https://github.com/maplemilesalt-byte/3GSEmu.git
cd 3GSEmu
cmake -S . -B build
cmake --build build --config Release
.\build\Release\3gs-emu.exe
```

## Dependencies

- CMake 3.16+
- A C++20 compiler
- Git

No third-party libraries are required for the current CPU prototype.

## Current status

The current prototype contains:

- ARMv7 CPU interpreter
- ARM data-processing instructions
- ARM branches
- ARM LDR/STR
- ARM condition codes
- Thumb execution basics
- 16 MiB prototype RAM

The memory size and device model are temporary and do not represent the final iPhone 3GS hardware map yet.

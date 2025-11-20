# Build Instructions

## Prerequisites

- CMake 3.5 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

## Quick Start

### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./debugger
```

### Windows (Visual Studio)

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Run
.\Release\debugger.exe
```

## Build Options

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build (optimized)

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Enable 65C02 Emulation Mode

Edit `CMakeLists.txt` and uncomment:

```cmake
target_compile_definitions(cpu65816 PRIVATE EMU_65C02)
```

## Project Structure

```
debugger/
├── CMakeLists.txt          # Main build configuration
├── BUILD.md                # This file
├── build/                  # Build artifacts (created by you)
└── src/
    ├── main.cpp            # Application entry point
    ├── cpu/
    │   ├── include/        # CPU header files
    │   ├── opcodes/        # OpCode implementations
    │   └── *.cpp           # CPU source files
    ├── devices/
    │   ├── Ram.hpp
    │   └── Ram.cpp
    └── logger/
        ├── Log.hpp
        └── Log.cpp
```

## Libraries Built

1. **logger** - Logging functionality
2. **cpu65816** - 65816/65C02 CPU emulation
3. **devices** - Hardware devices (RAM, etc.)
4. **debugger** - Main executable (links all libraries)

## Cleaning Build

```bash
# From project root
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

## Troubleshooting

### Compilation Errors

If you encounter compilation errors, verify:

1. All header files are in `src/cpu/include/`
2. All CPU implementation files are in `src/cpu/` or `src/cpu/opcodes/`
3. You're using a C++17 compatible compiler
# RealmEngine Build Scripts

[English](GUIDE.md) | [中文](GUIDE_zh.md)

Cross-platform build system for RealmEngine supporting Windows, Linux, and macOS.

## Overview

The build system consists of modular Python scripts located in the `scripts/` directory:

- **build.py** - Main build script with cross-platform support
- **build_config.py** - Configuration and utilities module
- **format.py** - Code formatting using clang-format
- **lint.py** - Code linting using clang-tidy
- **clean.py** - Clean build artifacts
- **test.py** - Test runner

## Requirements

### All Platforms
- Python 3.6+
- CMake 3.20+
- C++17 compatible compiler

### Platform-Specific

**Windows:**
- Visual Studio 2017+ (MSVC) or MinGW
- Optional: Ninja build system

**Linux:**
- GCC 7+ or Clang 5+
- Optional: Ninja build system
- Optional: clang-format, clang-tidy

**macOS:**
- Xcode Command Line Tools
- Optional: Ninja build system
- Optional: clang-format, clang-tidy

## Quick Start

### Basic Build

```bash
# Default build (Debug)
python build.py

# Release build
python build.py -t Release

# Build and run
python build.py -r

# Clean build
python build.py -c
```

### Build Options

```bash
# Show all options
python build.py --help

# Configure only (don't build)
python build.py --configure

# Build only (skip configure)
python build.py --build

# Specify build directory
python build.py -d build-release

# Specify number of parallel jobs
python build.py -j 8

# Build specific target
python build.py -T RealmEngine

# Verbose output
python build.py -v

# Pass CMake defines
python build.py -D CMAKE_CXX_COMPILER=clang++
```

### Build Types

- **Debug** - Debug build with symbols (default)
- **Release** - Optimized release build
- **RelWithDebInfo** - Release with debug info
- **MinSizeRel** - Minimum size release

## Code Quality Tools

### Format Code

```bash
# Format all source files
python scripts/format.py

# Check formatting without modifying
python scripts/format.py --check

# Format specific directory
python scripts/format.py -d src/core

# Verbose output
python scripts/format.py -v
```

### Lint Code

```bash
# Lint all source files
python scripts/lint.py

# Lint with auto-fix
python scripts/lint.py --fix

# Lint specific directory
python scripts/lint.py -d src/renderer

# Use specific checks
python scripts/lint.py -c "modernize-*,readability-*"

# Verbose output
python scripts/lint.py -v
```

**Note:** Linting requires `compile_commands.json`. Generate it with:
```bash
python build.py --configure
```
Generate only clangd completion data (configure, no build):
```bash
python scripts/clangd.py
```
To use the VS (MSVC) toolchain: run the above from "x64 Native Tools Command Prompt for VS" or "Developer Command Prompt for VS"; the script uses Ninja and picks up `cl.exe` from the environment, so the generated `compile_commands.json` matches your VS build.

## Clean Build Artifacts

```bash
# Clean build directory
python scripts/clean.py

# Clean all (build + bin + cache)
python scripts/clean.py --all

# Clean specific directories
python scripts/clean.py --build
python scripts/clean.py --bin
python scripts/clean.py --cache

# Dry run (show what would be removed)
python scripts/clean.py --all --dry-run
```

## Run Tests

```bash
# Run all tests
python scripts/test.py

# Run Release build tests
python scripts/test.py -t Release

# Verbose output
python scripts/test.py -v
```

## Platform-Specific Notes

### Windows

The build system automatically detects and uses Visual Studio. If you prefer MinGW or Ninja:

```bash
# Use MinGW Makefiles
python build.py -g "MinGW Makefiles"

# Use Ninja (requires ninja in PATH)
python build.py -g Ninja
```

### Linux

The build system prefers Ninja if available, falling back to Make:

```bash
# Force Unix Makefiles
python build.py -g "Unix Makefiles"

# Install Ninja (Ubuntu/Debian)
sudo apt install ninja-build

# Install clang tools (Ubuntu/Debian)
sudo apt install clang-format clang-tidy
```

### macOS

Similar to Linux, Ninja is preferred:

```bash
# Install tools via Homebrew
brew install cmake ninja llvm

# Use LLVM clang-format and clang-tidy
python scripts/format.py --clang-format /usr/local/opt/llvm/bin/clang-format
python scripts/lint.py --clang-tidy /usr/local/opt/llvm/bin/clang-tidy
```

## Common Workflows

### Full Clean Build and Run

```bash
python build.py -c -r
```

### Release Build for Distribution

```bash
python build.py -t Release -j 8
```

### Development with Code Quality

```bash
# Format code
python scripts/format.py

# Build
python build.py

# Lint (requires previous build)
python scripts/lint.py

# Run
python build.py -r
```

### CI/CD Pipeline

```bash
# Check formatting
python scripts/format.py --check

# Build
python build.py -t Release

# Lint
python scripts/lint.py

# Run tests
python scripts/test.py -t Release
```

## Advanced Usage

### Custom CMake Options

```bash
# Enable/disable features
python build.py -D BUILD_TESTS=ON -D BUILD_DOCS=OFF

# Use custom compiler
python build.py -D CMAKE_CXX_COMPILER=clang++

# Set custom install prefix
python build.py -D CMAKE_INSTALL_PREFIX=/opt/realmengine
```

### Multiple Build Configurations

```bash
# Debug build
python build.py -d build-debug -t Debug

# Release build
python build.py -d build-release -t Release

# Run specific build
cd bin
./RealmEngine  # Or RealmEngine.exe on Windows
```

### Parallel Builds

```bash
# Use all CPU cores
python build.py -j $(nproc)  # Linux/macOS
python build.py -j %NUMBER_OF_PROCESSORS%  # Windows

# Limit to 4 cores
python build.py -j 4
```

## Troubleshooting

### CMake Not Found

**Error:** `CMake not found`

**Solution:** Install CMake from https://cmake.org/download/

### Compiler Not Found

**Error:** `Missing required build tools: g++ or clang++`

**Windows:** Install Visual Studio or MinGW
**Linux:** `sudo apt install build-essential`
**macOS:** `xcode-select --install`

### Ninja Not Found

**Error:** `Ninja not found, falling back to Unix Makefiles`

**Solution:** This is a warning, not an error. Install Ninja for faster builds:
- Windows: Download from https://ninja-build.org/
- Linux: `sudo apt install ninja-build`
- macOS: `brew install ninja`

### compile_commands.json Not Found

**Error:** `compile_commands.json not found`

**Solution:** Configure the project first:
```bash
python build.py --configure
```

### Encoding Issues (Windows)

If you see encoding errors on Windows, the scripts automatically handle encoding issues. If problems persist, try:

```bash
# Set UTF-8 encoding
chcp 65001
python build.py
```

## Directory Structure

```
RealmEngine/
├── build.py              # Main entry point (calls scripts/build.py)
├── scripts/              # Build scripts directory
│   ├── build.py          # Main build script
│   ├── build_config.py   # Configuration module
│   ├── format.py         # Code formatting
│   ├── lint.py           # Code linting
│   ├── clean.py          # Clean artifacts
│   └── test.py           # Test runner
├── src/                  # Source code
├── build/                # Build artifacts (generated)
├── bin/                  # Output binaries (generated)
└── CMakeLists.txt        # CMake configuration
```

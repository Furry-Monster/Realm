# RealmEngine

[English](README.md) | [中文](README_zh.md)

A modern game engine based on OpenGL, featuring a PBR rendering pipeline, an integrated visual editor, and an ECS architecture.

![editor](docs/editor.png "editor demo")

![pbr + npr](docs/aqua.png "pbr demo")

![pbr + npr](docs/example.png)

![debug](docs/debug.png)

## Features

- **Rendering Pipeline**  — Abstraction of OpenGL RHI, supporting a Metallic/Roughness PBR workflow based on the Cook-Torrance BRDF.
- **Lighting & Shading** — Supports IBL (Image-Based Lighting), SSAO (Screen-Space Ambient Occlusion), Screen-Space Subsurface Scattering (SSSS), and Kajiya-Kay hair rendering.
- **Visual Editor** — An ImGui-powered editor featuring scene editing, entity management, property inspection, asset management, and engine parameter hot-swapping.
- **Entity Component System (ECS)** — A component-based architecture with built-in support for Transform, Renderable, Lighting, CameraController, and more.
- **Scene Management** — Scene graph-based management supporting creation, loading, saving, synchronization, and serialization.
- **Resource Management** — Asset importing for glTF, FBX, and other major 3D formats.

## Requirements

- **OS**: Windows / Linux / macOS
- **Compiler**: C++17
  - Windows: Visual Studio 2017+ or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools
- **CMake**: 3.20+
- **OpenGL**: 3.3+
- **Python**: 3.6+ (build scripts)

## Dependencies

Dependencies are in `libs/`:

- **GLFW** — Window and input
- **GLAD** — OpenGL loader
- **GLM** — Math
- **Assimp** — Model loading
- **spdlog** — Logging
- **ImGui** — Immediate-mode GUI
- **stb** — Image loading
- **EnTT** — Entity Component System

Managed as submodules. Clone with:

```bash
# Option 1: Recursive clone
git clone --recursive https://github.com/Furry-Monster/RealmEngine

# Option 2: Step by step
git clone https://github.com/Furry-Monster/RealmEngine
cd RealmEngine
git submodule init
git submodule update
```

## Quick Start

### Build Script (Recommended)

Cross-platform Python build script:

```bash
# Default Debug build
python scripts/build.py

# Release build
python scripts/build.py -t Release

# Clean and build
python scripts/build.py -c

# Build and run
python scripts/build.py -r

# Specify parallel jobs
python scripts/build.py -j 8
```

**Platform shortcuts:**

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

After build, run `bin/RealmEngine` (or `bin/RealmEngine.exe` on Windows) to launch the editor.

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# Run (Linux/macOS)
../bin/RealmEngine
```

## Build Options

### Build Types

- `Debug` — Debug (default)
- `Release` — Release optimization
- `RelWithDebInfo` — Release with debug info
- `MinSizeRel` — Minimum size

### Build Script Arguments

```bash
python scripts/build.py --help

Common options:
  -t, --type TYPE        Build type
  -d, --dir DIR          Build directory (default: build)
  -g, --generator GEN    CMake generator
  -j, --jobs N           Parallel jobs
  -c, --clean            Clean
  -r, --run              Run after build
  -v, --verbose          Verbose output
  --configure            Configure only
  --build                Build only
  -D VAR=VALUE           Pass CMake variables
```

### Utility Scripts

```bash
# Format code
python scripts/format.py
python scripts/format.py --check

# Lint
python scripts/lint.py
python scripts/lint.py --fix

# Clean
python scripts/clean.py
python scripts/clean.py --all

# Test
python scripts/test.py
```

See `scripts/GUIDE.md` for details.

## License
This project is licensed under the MIT License - See [LICENSE](LICENSE) in the project root.

## Contributing

Issues and Pull Requests are welcome.

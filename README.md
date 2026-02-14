# RealmEngine

[English](README.md) | [中文](README_zh.md)

Modern OpenGL-based game engine with PBR pipeline, IBL lighting, and Bloom post-processing. Integrated visual editor and Entity-Component-System (ECS).

![pbr demo](helmet.png "pbr demo")

![editor demo](editor.png "editor demo")

![img](debug.png)

## Features

- **Physically Based Rendering (PBR)** — Metallic/Roughness workflow
- **Image-Based Lighting (IBL)** — HDR environment maps and precomputed lighting
- **Bloom Post-Processing** — Configurable bloom, tone mapping, and gamma correction
- **Visual Editor** — ImGui-based editor for scene editing, entity management, and property modification
- **Entity-Component-System (ECS)** — Component-based architecture with Transform, Renderable, Lighting, CameraController, etc.
- **Scene Management** — Create, load, save, and serialize scenes
- **Resource Management** — glTF, FBX, and other model formats
- **Configuration** — Unified config with serialization and encryption
- **Window Management** — Cross-platform GLFW-based window
- **Logging** — spdlog-based logging

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
python build.py

# Release build
python build.py -t Release

# Clean and build
python build.py -c

# Build and run
python build.py -r

# Specify parallel jobs
python build.py -j 8
```

**Platform shortcuts:**

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

After build, run `bin/RealmEngine` (or `bin/RealmEngine.exe` on Windows) to launch the editor.

Debug mode (no editor UI):

```bash
python build.py -r -- --debug
```

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# Run (Linux/macOS)
../bin/RealmEngine
```

## Directory Structure

```
RealmEngine/
├── build.py         # Build entry
├── build.sh         # Unix build script
├── build.bat        # Windows build script
├── scripts/         # Build scripts
│   ├── build.py
│   ├── build_config.py
│   ├── format.py
│   ├── lint.py
│   ├── clean.py
│   ├── test.py
│   ├── GUIDE.md
│   └── QUICKREF.md
├── assets/          # Resources (models, textures, HDR, etc.)
├── shaders/         # GLSL shaders
├── src/             # Source
│   ├── main.cpp
│   ├── editor/      # Editor
│   ├── render/      # Rendering
│   ├── resource/    # Resource management
│   ├── gameplay/    # Game logic
│   │   ├── components/
│   │   └── scene/
│   └── ...
├── libs/            # Third-party libs
├── bin/             # Output
└── build/           # CMake build dir
```

## Build Options

### Build Types

- `Debug` — Debug (default)
- `Release` — Release optimization
- `RelWithDebInfo` — Release with debug info
- `MinSizeRel` — Minimum size

### Build Script Arguments

```bash
python build.py --help

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

See `scripts/GUIDE.md` and `scripts/QUICKREF.md` for details.

## Editor

Editor panels:

- **Menu Bar** — File (new, open, save scene), Edit, etc.
- **Scene Hierarchy** — Entity and node list with selection and operations
- **Property Panel** — Edit components (Transform, Renderable, Lighting, etc.) of selected entity
- **Viewport** — Real-time scene preview
- **File Dialog** — Open and save scenes

Scenes are stored in JSON with full serialization/deserialization.

## Resources

- **Models**: glTF, FBX, etc. (Assimp)
- **Textures**: Common formats (stb_image)
- **HDR Environment**: .hdr (IBL)

Place resources in `assets/`. They are copied to `bin/assets/` during build.

## Shaders

`shaders/` directory:

- `pbr.vert/frag` — PBR main shader
- `skybox.vert/frag` — Skybox
- `bloom.vert/frag` — Bloom
- `post.vert/frag` — Post-processing
- `ibl/` — IBL related

## License

See [LICENSE](LICENSE) in the project root.

## Contributing

Issues and Pull Requests are welcome.

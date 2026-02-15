<p align="center">
  <h1 align="center">RealmEngine</h1>
  <p align="center">
    A modern OpenGL game engine with PBR rendering, a visual editor, and an ECS architecture.
    <br />
    <a href="README.md">English</a> | <a href="README_zh.md">中文</a>
  </p>
</p>

---

## Gallery

| Editor                   | PBR Rendering             |
| ------------------------ | ------------------------- |
| ![Editor](docs/editor.png) | ![PBR + NPR](docs/aqua.png) |

| Scene Example              | Debug View             |
| -------------------------- | ---------------------- |
| ![Example](docs/example.png) | ![Debug](docs/debug.png) |

## Overview

**RealmEngine** is a from-scratch 3D game engine built on OpenGL, designed for real-time rendering with a physically-based pipeline. It ships with a fully integrated ImGui-based visual editor and a flexible Entity Component System powered by EnTT.

### Key Highlights

- **Cook-Torrance PBR** with Metallic/Roughness workflow
- **Image-Based Lighting** (diffuse irradiance, specular prefiltering, BRDF LUT)
- **Multi-pass rendering** — Shadow, Geometry, SSAO, Bloom, SSS, Post-processing
- **Visual scene editor** with undo/redo, hotkeys, and real-time parameter tuning
- **RHI abstraction layer** — OpenGL backend with architecture ready for Vulkan/D3D12

## Features

### Rendering Pipeline

RealmEngine implements a multi-pass deferred-style rendering pipeline:

| Pass                   | Description                                                          |
| ---------------------- | -------------------------------------------------------------------- |
| **Shadow**       | Directional and point light shadow mapping (2048x2048)               |
| **Geometry**     | Main PBR pass — Cook-Torrance BRDF with metallic/roughness workflow |
| **Hair**         | Kajiya-Kay hair shading with shell-based layering                    |
| **SSAO**         | Screen-Space Ambient Occlusion with dedicated blur pass              |
| **SSS**          | Screen-Space Subsurface Scattering                                   |
| **Skybox**       | HDR environment skybox rendering                                     |
| **Bloom**        | Brightness extraction + Gaussian blur bloom                          |
| **Post-process** | Tonemapping, gamma correction, final compositing                     |

### Lighting & Shading

- **IBL** — Precomputed diffuse irradiance maps, specular prefiltered environment maps, and BRDF lookup textures
- **Light Types** — Directional, Point, Spot, Area
- **SSAO** — Screen-space ambient occlusion for contact shadows
- **SSS** — Screen-space subsurface scattering for skin/translucent materials
- **Hair** — Kajiya-Kay anisotropic hair rendering model

### Visual Editor

An ImGui-powered editor with docking support:

- **Viewport** — Real-time 3D scene preview with orbit/fly camera
- **Scene Hierarchy** — Tree view of entities with drag-and-drop
- **Properties Panel** — Per-component inspection and editing
- **Asset Browser** — Browse and manage project assets
- **Console** — Log output with filtering
- **Profiler** — Real-time performance metrics
- **Project Settings** — Engine configuration (SSAO, Bloom, SSS, tonemapping, etc.)
- **Preferences** — Editor theme and behavior settings
- **Command System** — Full undo/redo with command pattern
- **Hotkeys** — Configurable keyboard shortcuts

### Entity Component System

Built on [EnTT](https://github.com/skypjack/entt) for high-performance ECS:

- **Transform** — Position, rotation (quaternion), scale with hierarchy propagation
- **Renderable** — Mesh and material binding
- **Lighting** — Directional, Point, Spot, and Area light components
- **CameraController** — Orbital and fly-through camera control
- **Hierarchy** — Parent-child entity relationships
- **NameTag** — Human-readable entity names

Systems: `TransformSystem` (world transform updates), `HierarchySystem` (scene graph maintenance)

### Resource Management

- Supports **glTF**, **FBX**, **OBJ**, **PLY**, **STL** via Assimp
- Automatic texture caching with sRGB detection
- Model caching to avoid redundant loads
- JSON-based scene serialization and deserialization
- JSON-based engine configuration (`config.json`)

## Architecture

RealmEngine is structured as three build targets:

```
┌─────────────────────────────────────────────────────┐
│                   RealmEditor                       │
│         (ImGui editor, panels, commands)            │
├──────────────────────┬──────────────────────────────┤
│                      │       RealmRuntime           │
│                      │    (standalone player)       │
│                      ├──────────────────────────────┤
│                      │                              │
│              RealmEngineLib (static library)        │
│  ┌──────────┬──────────┬──────────┬───────────────┐ │
│  │   RHI    │ Renderer │  Scene   │   Resource    │ │
│  │ (OpenGL) │ (passes) │  (ECS)   │  (assets)     │ │
│  └──────────┴──────────┴──────────┴───────────────┘ │
│  ┌──────────┬──────────┬──────────┬───────────────┐ │
│  │ Platform │   Core   │  Event   │    Input      │ │
│  │ (window) │  (log)   │  (bus)   │  (keyboard)   │ │
│  └──────────┴──────────┴──────────┴───────────────┘ │
└─────────────────────────────────────────────────────┘
```

### Engine Subsystems (initialization order)

1. **EventBus** — Decoupled inter-subsystem communication
2. **Logger** — spdlog-based logging with console sink
3. **ConfigManager** — JSON configuration loading/saving
4. **AssetManager** — Model and texture caching
5. **SceneManager** — Scene lifecycle management
6. **Window** — GLFW window creation and context
7. **Renderer** — Multi-pass rendering pipeline
8. **Input** — Keyboard and mouse input polling

### RHI Abstraction

The Render Hardware Interface decouples rendering logic from the graphics API:

- `RHIDevice` — Abstract factory for GPU resources
- `RHIBuffer`, `RHITexture`, `RHIShader`, `RHIFramebuffer`, `RHIVertexInput`
- Current backend: **OpenGL** (`GLDevice`)
- Designed for future Vulkan / D3D12 backends

## Project Structure

```
RealmEngine/
├── engine/                 # Core engine (static library)
│   ├── core/               #   Logging, events, math, debug
│   ├── platform/           #   Window, input, filesystem
│   ├── rhi/                #   Render Hardware Interface
│   │   └── opengl/         #     OpenGL backend
│   ├── renderer/           #   Rendering pipeline
│   │   ├── passes/         #     Render passes (PBR, shadow, SSAO, bloom…)
│   │   └── ibl/            #     Image-Based Lighting
│   ├── resource/           #   Asset & config management
│   └── scene/              #   ECS, components, systems, serialization
├── editor/                 # Visual editor (executable)
│   ├── panels/             #   UI widgets (viewport, hierarchy, properties…)
│   ├── commands/           #   Undo/redo command system
│   ├── bridge/             #   Editor ↔ engine communication
│   ├── hotkey/             #   Keyboard shortcut manager
│   └── preferences/        #   Editor settings
├── runtime/                # Standalone runtime (executable)
├── shaders/                # GLSL shaders (PBR, shadow, SSAO, bloom, IBL…)
├── assets/                 # Models, textures, HDR environment maps
├── libs/                   # Third-party dependencies (git submodules)
├── scripts/                # Python build & quality tools
└── CMakeLists.txt          # Root build configuration
```

## Requirements

| Requirement            | Version                                 |
| ---------------------- | --------------------------------------- |
| **OS**           | Windows, Linux, or macOS                |
| **C++ Compiler** | MSVC 2017+, GCC 7+, or Clang 5+ (C++17) |
| **CMake**        | 3.20+                                   |
| **OpenGL**       | 3.3+                                    |
| **Python**       | 3.6+ (for build scripts)                |

## Dependencies

All dependencies are managed as git submodules in `libs/`:

| Library                                        | Purpose                                      |
| ---------------------------------------------- | -------------------------------------------- |
| [GLFW](https://www.glfw.org/)                     | Window creation and input handling           |
| [GLAD](https://glad.dav1d.de/)                    | OpenGL function loader                       |
| [GLM](https://github.com/g-truc/glm)              | Mathematics (vectors, matrices, quaternions) |
| [Assimp](https://www.assimp.org/)                 | 3D model importing (glTF, FBX, OBJ, etc.)    |
| [spdlog](https://github.com/gabime/spdlog)        | Fast logging                                 |
| [ImGui](https://github.com/ocornut/imgui)         | Immediate-mode GUI (docking branch)          |
| [stb](https://github.com/nothings/stb)            | Image loading (stb_image)                    |
| [EnTT](https://github.com/skypjack/entt)          | Entity Component System                      |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing                                 |

## Getting Started

### Clone

```bash
git clone --recursive https://github.com/Furry-Monster/RealmEngine.git
cd RealmEngine
```

If you already cloned without `--recursive`:

```bash
git submodule init && git submodule update
```

### Build (Recommended — Python Script)

```bash
# Debug build (default)
python scripts/build.py

# Release build with 8 parallel jobs
python scripts/build.py -t Release -j 8

# Clean, build, and run
python scripts/build.py -c -r
```

Platform shortcuts:

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

### Build (Manual — CMake)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### Run

```bash
# Launch the editor
./bin/RealmEditor          # Linux / macOS
bin\RealmEditor.exe        # Windows

# Launch the standalone runtime
./bin/RealmRuntime         # Linux / macOS
bin\RealmRuntime.exe       # Windows
```

## Build Options

| Option                  | Description                                                                     |
| ----------------------- | ------------------------------------------------------------------------------- |
| `-t, --type TYPE`     | Build type:`Debug` (default), `Release`, `RelWithDebInfo`, `MinSizeRel` |
| `-j, --jobs N`        | Number of parallel compile jobs                                                 |
| `-c, --clean`         | Clean before building                                                           |
| `-r, --run`           | Run after building                                                              |
| `-g, --generator GEN` | CMake generator (e.g.,`Ninja`, `"Unix Makefiles"`)                          |
| `-v, --verbose`       | Verbose build output                                                            |
| `-D VAR=VALUE`        | Pass arbitrary CMake variables                                                  |
| `--configure`         | Configure only (skip build)                                                     |
| `--build`             | Build only (skip configure)                                                     |

### Code Quality Tools

```bash
# Format source code (clang-format)
python scripts/format.py
python scripts/format.py --check      # Check only, no modifications

# Static analysis (clang-tidy)
python scripts/lint.py
python scripts/lint.py --fix          # Auto-fix issues

# Clean build artifacts
python scripts/clean.py --all

# Run tests
python scripts/test.py
```

See [`scripts/GUIDE.md`](scripts/GUIDE.md) for full build system documentation.

## Configuration

RealmEngine uses a JSON configuration file (`bin/config.json`) auto-generated on first run:

| Section            | Settings                                                    |
| ------------------ | ----------------------------------------------------------- |
| **General**  | Asset paths, shader directory                               |
| **Window**   | Resolution, title, fullscreen, VSync, MSAA                  |
| **Renderer** | Camera, SSAO, Bloom, SSS, tonemapping, HDRI environment map |
| **Gameplay** | Camera speed, mouse sensitivity, default scene file         |

All renderer parameters can be tweaked in real-time through the editor's Project Settings panel.

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Feel free to open an [Issue](https://github.com/Furry-Monster/RealmEngine/issues) or submit a [Pull Request](https://github.com/Furry-Monster/RealmEngine/pulls).

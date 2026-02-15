<p align="center">
  <h1 align="center">RealmEngine</h1>
  <p align="center">
    基于 OpenGL 的现代游戏引擎，集成 PBR 渲染管线、可视化编辑器与 ECS 架构。
    <br />
    <a href="README.md">English</a> | <a href="README_zh.md">中文</a>
  </p>
</p>

---

## 效果展示

| 编辑器                   | PBR 渲染                  |
| ------------------------ | ------------------------- |
| ![编辑器](docs/editor.png) | ![PBR + NPR](docs/aqua.png) |

| 场景示例                | 调试视图              |
| ----------------------- | --------------------- |
| ![示例](docs/example.png) | ![调试](docs/debug.png) |

## 概述

**RealmEngine** 是一款从零构建的 3D 游戏引擎，基于 OpenGL 实现实时物理渲染管线。引擎内置基于 ImGui 的可视化编辑器，以及基于 EnTT 的高性能实体组件系统。

### 核心亮点

- **Cook-Torrance PBR** — 金属度/粗糙度工作流
- **基于图像的光照 (IBL)** — 漫反射辐照度图、高光预过滤环境贴图、BRDF 查找表
- **多 Pass 渲染** — 阴影、几何、SSAO、泛光、次表面散射、后处理
- **可视化场景编辑器** — 支持撤销/重做、快捷键、实时参数调节
- **RHI 抽象层** — 当前 OpenGL 后端，架构预留 Vulkan/D3D12 扩展

## 特性详解

### 渲染管线

RealmEngine 实现了多 Pass 延迟风格渲染管线：

| 渲染 Pass              | 说明                                                         |
| ---------------------- | ------------------------------------------------------------ |
| **Shadow**       | 方向光与点光源阴影映射（2048x2048）                          |
| **Geometry**     | 主 PBR Pass — 基于 Cook-Torrance BRDF 的金属度/粗糙度工作流 |
| **Hair**         | Kajiya-Kay 毛发着色 + Shell 分层渲染                         |
| **SSAO**         | 屏幕空间环境光遮蔽（含专用模糊 Pass）                        |
| **SSS**          | 屏幕空间次表面散射                                           |
| **Skybox**       | HDR 环境天空盒渲染                                           |
| **Bloom**        | 亮度提取 + 高斯模糊泛光                                      |
| **Post-process** | 色调映射、Gamma 校正、最终合成                               |

### 光照与着色

- **IBL** — 预计算漫反射辐照度贴图、高光预过滤环境贴图、BRDF 查找纹理
- **光源类型** — 方向光、点光源、聚光灯、面光源
- **SSAO** — 屏幕空间环境光遮蔽，增强接触阴影
- **SSS** — 屏幕空间次表面散射，适用于皮肤/半透明材质
- **Hair** — Kajiya-Kay 各向异性毛发渲染模型

### 可视化编辑器

基于 ImGui（Docking 分支）的编辑器：

- **视口** — 实时 3D 场景预览，支持轨道/自由飞行摄像机
- **场景层级** — 实体树形视图，支持拖放操作
- **属性面板** — 逐组件检视与编辑
- **资产浏览器** — 浏览和管理项目资产
- **控制台** — 日志输出与过滤
- **性能分析器** — 实时性能指标
- **项目设置** — 引擎参数配置（SSAO、Bloom、SSS、色调映射等）
- **偏好设置** — 编辑器主题与行为设置
- **命令系统** — 基于命令模式的撤销/重做
- **快捷键** — 可配置的键盘快捷方式

### 实体组件系统

基于 [EnTT](https://github.com/skypjack/entt) 构建的高性能 ECS：

- **Transform** — 位置、旋转（四元数）、缩放，支持层级传播
- **Renderable** — 网格与材质绑定
- **Lighting** — 方向光、点光源、聚光灯、面光源组件
- **CameraController** — 轨道和自由飞行摄像机控制
- **Hierarchy** — 父子实体关系
- **NameTag** — 可读的实体名称

系统：`TransformSystem`（世界变换更新）、`HierarchySystem`（场景图维护）

### 资源管理

- 通过 Assimp 支持 **glTF**、**FBX**、**OBJ**、**PLY**、**STL**
- 自动纹理缓存与 sRGB 检测
- 模型缓存避免重复加载
- 基于 JSON 的场景序列化与反序列化
- 基于 JSON 的引擎配置（`config.json`）

## 架构设计

RealmEngine 由三个构建目标组成：

```
┌─────────────────────────────────────────────────────┐
│                   RealmEditor                       │
│           (ImGui 编辑器、面板、命令系统)                │
├──────────────────────┬──────────────────────────────┤
│                      │       RealmRuntime           │
│                      │      (独立运行时)              │
│                      ├──────────────────────────────┤
│                      │                              │
│              RealmEngineLib (静态库)                 │
│  ┌──────────┬──────────┬──────────┬───────────────┐ │
│  │   RHI    │ Renderer │  Scene   │   Resource    │ │
│  │ (OpenGL) │ (渲染Pass)│  (ECS)   │   (资产)      │ │
│  └──────────┴──────────┴──────────┴───────────────┘ │
│  ┌──────────┬──────────┬──────────┬───────────────┐ │
│  │ Platform │   Core   │  Event   │    Input      │ │
│  │  (窗口)   │  (日志)  │ (事件总线) │  (键鼠输入)    │ │
│  └──────────┴──────────┴──────────┴───────────────┘ │
└─────────────────────────────────────────────────────┘
```

### 引擎子系统（初始化顺序）

1. **EventBus** — 解耦的子系统间通信
2. **Logger** — 基于 spdlog 的日志系统
3. **ConfigManager** — JSON 配置加载/保存
4. **AssetManager** — 模型与纹理缓存
5. **SceneManager** — 场景生命周期管理
6. **Window** — GLFW 窗口创建与上下文
7. **Renderer** — 多 Pass 渲染管线
8. **Input** — 键盘与鼠标输入轮询

### RHI 抽象层

渲染硬件接口将渲染逻辑与图形 API 解耦：

- `RHIDevice` — GPU 资源的抽象工厂
- `RHIBuffer`、`RHITexture`、`RHIShader`、`RHIFramebuffer`、`RHIVertexInput`
- 当前后端：**OpenGL**（`GLDevice`）
- 架构设计兼容未来的 Vulkan / D3D12 后端

## 项目结构

```
RealmEngine/
├── engine/                 # 引擎核心（静态库）
│   ├── core/               #   日志、事件、数学、调试
│   ├── platform/           #   窗口、输入、文件系统
│   ├── rhi/                #   渲染硬件接口
│   │   └── opengl/         #     OpenGL 后端
│   ├── renderer/           #   渲染管线
│   │   ├── passes/         #     渲染 Pass（PBR、阴影、SSAO、泛光……）
│   │   └── ibl/            #     基于图像的光照
│   ├── resource/           #   资产与配置管理
│   └── scene/              #   ECS、组件、系统、序列化
├── editor/                 # 可视化编辑器（可执行文件）
│   ├── panels/             #   UI 面板（视口、层级、属性……）
│   ├── commands/           #   撤销/重做命令系统
│   ├── bridge/             #   编辑器 ↔ 引擎通信层
│   ├── hotkey/             #   快捷键管理器
│   └── preferences/        #   编辑器设置
├── runtime/                # 独立运行时（可执行文件）
├── shaders/                # GLSL 着色器（PBR、阴影、SSAO、泛光、IBL……）
├── assets/                 # 模型、纹理、HDR 环境贴图
├── libs/                   # 第三方依赖（git 子模块）
├── scripts/                # Python 构建与质量工具
└── CMakeLists.txt          # 根构建配置
```

## 环境要求

| 要求                 | 版本                                    |
| -------------------- | --------------------------------------- |
| **操作系统**   | Windows、Linux 或 macOS                 |
| **C++ 编译器** | MSVC 2017+、GCC 7+ 或 Clang 5+（C++17） |
| **CMake**      | 3.20+                                   |
| **OpenGL**     | 3.3+                                    |
| **Python**     | 3.6+（构建脚本）                        |

## 依赖

所有依赖以 git 子模块方式管理，位于 `libs/` 目录：

| 库                                             | 用途                             |
| ---------------------------------------------- | -------------------------------- |
| [GLFW](https://www.glfw.org/)                     | 窗口创建与输入处理               |
| [GLAD](https://glad.dav1d.de/)                    | OpenGL 函数加载                  |
| [GLM](https://github.com/g-truc/glm)              | 数学运算（向量、矩阵、四元数）   |
| [Assimp](https://www.assimp.org/)                 | 3D 模型导入（glTF、FBX、OBJ 等） |
| [spdlog](https://github.com/gabime/spdlog)        | 高性能日志                       |
| [ImGui](https://github.com/ocornut/imgui)         | 即时模式 GUI（Docking 分支）     |
| [stb](https://github.com/nothings/stb)            | 图像加载（stb_image）            |
| [EnTT](https://github.com/skypjack/entt)          | 实体组件系统                     |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON 解析                        |

## 快速开始

### 克隆

```bash
git clone --recursive https://github.com/Furry-Monster/RealmEngine.git
cd RealmEngine
```

如果已经克隆但未使用 `--recursive`：

```bash
git submodule init && git submodule update
```

### 构建（推荐 — Python 脚本）

```bash
# Debug 构建（默认）
python scripts/build.py

# Release 构建，8 线程并行
python scripts/build.py -t Release -j 8

# 清理、构建并运行
python scripts/build.py -c -r
```

平台快捷脚本：

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

### 构建（手动 — CMake）

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### 运行

```bash
# 启动编辑器
./bin/RealmEditor          # Linux / macOS
bin\RealmEditor.exe        # Windows

# 启动独立运行时
./bin/RealmRuntime         # Linux / macOS
bin\RealmRuntime.exe       # Windows
```

## 构建选项

| 选项                    | 说明                                                                         |
| ----------------------- | ---------------------------------------------------------------------------- |
| `-t, --type TYPE`     | 构建类型：`Debug`（默认）、`Release`、`RelWithDebInfo`、`MinSizeRel` |
| `-j, --jobs N`        | 并行编译任务数                                                               |
| `-c, --clean`         | 构建前清理                                                                   |
| `-r, --run`           | 构建后运行                                                                   |
| `-g, --generator GEN` | CMake 生成器（如 `Ninja`、`"Unix Makefiles"`）                           |
| `-v, --verbose`       | 详细构建输出                                                                 |
| `-D VAR=VALUE`        | 传递 CMake 变量                                                              |
| `--configure`         | 仅配置（跳过构建）                                                           |
| `--build`             | 仅构建（跳过配置）                                                           |

### 代码质量工具

```bash
# 格式化源代码（clang-format）
python scripts/format.py
python scripts/format.py --check      # 仅检查，不修改

# 静态分析（clang-tidy）
python scripts/lint.py
python scripts/lint.py --fix          # 自动修复

# 清理构建产物
python scripts/clean.py --all

# 运行测试
python scripts/test.py
```

完整构建系统文档请参阅 [`scripts/GUIDE_zh.md`](scripts/GUIDE_zh.md)。

## 配置

RealmEngine 使用 JSON 配置文件（`bin/config.json`），首次运行时自动生成：

| 配置项             | 内容                                              |
| ------------------ | ------------------------------------------------- |
| **General**  | 资产路径、着色器目录                              |
| **Window**   | 分辨率、标题、全屏、VSync、MSAA                   |
| **Renderer** | 摄像机、SSAO、Bloom、SSS、色调映射、HDRI 环境贴图 |
| **Gameplay** | 摄像机速度、鼠标灵敏度、默认场景文件              |

所有渲染参数均可在编辑器的「项目设置」面板中实时调节。

## 许可证

本项目基于 **MIT 许可证** 开源 — 详见 [LICENSE](LICENSE)。

## 贡献

欢迎贡献！请随时提交 [Issue](https://github.com/Furry-Monster/RealmEngine/issues) 或 [Pull Request](https://github.com/Furry-Monster/RealmEngine/pulls)。

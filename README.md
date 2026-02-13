# RealmEngine

一个基于 OpenGL 的现代游戏引擎，专注于基于物理的渲染（PBR）和高质量图形效果，支持IBL与Bloom后处理。现已包含完整的编辑器系统和实体组件系统（EC）。

![pbr demo](helmet.png "pbr demo")

![编辑器demo](editor.png "editor demo")

## 特性

- **基于物理的渲染（PBR）** - 支持金属度/粗糙度工作流
- **基于图像的照明（IBL）** - 支持 HDR 环境贴图和预计算光照
- **Bloom 后处理** - 可配置的泛光效果、色调映射与伽马矫正
- **可视化编辑器** - 基于 ImGui 的完整编辑器，支持场景编辑、实体管理和属性调整
- **实体组件系统（ECS）** - 灵活的组件系统，支持 Transform、Renderable、Lighting、CameraController 等组件
- **场景管理** - 完整的场景创建、加载、保存和管理系统，支持场景序列化
- **资源管理** - 支持 glTF、FBX 等模型格式
- **配置管理** - 统一的配置管理系统，支持序列化和加密
- **窗口管理** - 基于 GLFW 的跨平台窗口系统
- **日志系统** - 基于 spdlog 的日志记录

## 系统要求

- **操作系统**: Windows / Linux / macOS（跨平台支持）
- **编译器**: 支持 C++17 的编译器
  - Windows: Visual Studio 2017+ 或 MinGW
  - Linux: GCC 7+ 或 Clang 5+
  - macOS: Xcode Command Line Tools
- **CMake**: 3.20 或更高版本
- **OpenGL**: 3.3 或更高版本
- **Python**: 3.6+（用于构建脚本）

## 依赖库

项目已包含以下依赖库（位于 `libs/` 目录）：

- **GLFW** - 窗口和输入管理
- **GLAD** - OpenGL 加载器
- **GLM** - 数学库
- **Assimp** - 模型加载
- **spdlog** - 日志系统
- **ImGui** - 即时模式 GUI
- **stb** - 图像加载

项目中依赖库大部分使用submodule管理，克隆时请同时克隆依赖库，例如：

```bash
# 1.使用递归克隆
git clone --recursive https://github.com/Furry-Monster/Realm

# 2.手动管理依赖库
# 克隆原始仓库
git clone https://github.com/Furry-Monster/Realm

# 初始化子模块
git submodule init

# 下载依赖库
git submodule update
```

## 快速开始

### 使用构建脚本（推荐）

项目提供了跨平台的 Python 构建脚本，支持 Windows、Linux 和 macOS：

```bash
# 默认构建（Debug 模式）
python build.py

# Release 模式构建
python build.py -t Release

# 清理并构建
python build.py -c

# 构建并运行
python build.py -r

# 使用 8 个并行任务构建
python build.py -j 8
```

**平台特定的便捷脚本：**

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

构建完成后，运行 `bin/RealmEngine`（Windows 下为 `bin/RealmEngine.exe`）将启动编辑器模式。

如需运行调试模式（无编辑器界面），传递参数：
```bash
python build.py -r -- --debug
```

### 手动构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build . -j$(nproc)

# 运行（Linux/macOS）
../bin/RealmEngine
```

## 项目结构

```
RealmEngine/
├── build.py         # 主构建脚本入口
├── build.sh         # Unix 构建脚本（Linux/macOS）
├── build.bat        # Windows 构建脚本
├── scripts/         # 构建脚本目录
│   ├── build.py         # 主构建脚本
│   ├── build_config.py  # 构建配置模块
│   ├── format.py        # 代码格式化
│   ├── lint.py          # 代码检查
│   ├── clean.py         # 清理脚本
│   ├── test.py          # 测试运行器
│   ├── README.md        # 构建脚本完整文档
│   └── QUICKREF.md      # 快速参考
├── assets/          # 资源文件（模型、纹理、HDR 等）
├── shaders/         # GLSL 着色器文件
├── src/             # 源代码
│   ├── main.cpp     # 引擎主循环
│   ├── editor/      # 编辑器系统
│   ├── render/      # 渲染系统
│   ├── resource/    # 资源管理
│   ├── gameplay/    # 游戏逻辑
│   │   ├── components/  # 组件系统
│   │   └── scene/       # 场景管理
│   └── ...
├── libs/            # 第三方库
├── bin/             # 构建输出目录
└── build/           # CMake 构建目录
```

## 构建选项

### 构建类型

- `Debug` - 调试模式（默认）
- `Release` - 发布模式（完全优化）
- `RelWithDebInfo` - 带调试信息的发布模式
- `MinSizeRel` - 最小体积发布模式

### 构建脚本选项

```bash
# 查看所有选项
python build.py --help

# 常用选项
python build.py [选项]

主要选项：
  -t, --type TYPE        构建类型 (Debug/Release/RelWithDebInfo/MinSizeRel)
  -d, --dir DIR          构建目录（默认: build）
  -g, --generator GEN    CMake 生成器（自动检测）
  -j, --jobs N           并行编译任务数（默认：CPU 核心数）
  -c, --clean            清理构建目录
  -r, --run              构建后运行
  -v, --verbose          详细输出
  --configure            仅配置 CMake
  --build                仅构建（跳过配置）
  -D VAR=VALUE           传递 CMake 定义
```

### 其他构建工具

```bash
# 代码格式化
python scripts/format.py              # 格式化所有代码
python scripts/format.py --check      # 检查格式不修改

# 代码检查
python scripts/lint.py                # 运行 clang-tidy
python scripts/lint.py --fix          # 自动修复问题

# 清理构建产物
python scripts/clean.py               # 清理 build 目录
python scripts/clean.py --all         # 清理所有生成文件

# 运行测试
python scripts/test.py
```

详细文档请参阅 `scripts/README.md` 和 `scripts/QUICKREF.md`。

## 编辑器功能

编辑器提供了以下功能面板：

- **菜单栏** - 文件操作（新建、打开、保存场景）、编辑功能等
- **场景层次结构** - 显示场景中的所有实体和节点，支持选择和操作
- **属性面板** - 显示和编辑选中实体的组件属性（Transform、Renderable、Lighting 等）
- **视口面板** - 实时预览场景渲染结果
- **文件对话框** - 用于打开和保存场景文件

场景文件以 JSON 格式保存，支持完整的场景序列化和反序列化。

## 资源文件

- **模型**: 支持 glTF、FBX 等格式（通过 Assimp）
- **纹理**: 支持常见图像格式（通过 stb_image）
- **HDR 环境贴图**: 支持 .hdr 格式用于 IBL

资源文件应放置在 `assets/` 目录中，构建时会自动复制到 `bin/assets/`。

## 着色器

着色器文件位于 `shaders/` 目录：

- `pbr.vert/frag` - PBR 主着色器
- `skybox.vert/frag` - 天空盒着色器
- `bloom.vert/frag` - Bloom 后处理着色器
- `post.vert/frag` - 后处理着色器
- `ibl/` - IBL 相关着色器

## 贡献

欢迎提交 Issue 和 Pull Request！

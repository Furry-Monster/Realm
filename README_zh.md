# RealmEngine

[English](README.md) | [中文](README_zh.md)

基于 OpenGL 的现代游戏引擎，采用PBR渲染管线，集成可视化编辑器与ECS。

![编辑器demo](docs/editor.png "editor demo")

![pbr + npr](docs/aqua.png "pbr demo")

![pbr + npr](docs/example.png)

![调试](docs/debug.png)

## 核心特性

- **渲染管线** — 封装OpenGL RHI，基于Cook-Torrance BRDF的金属度 / 粗糙度 PBR 工作流
- **光照与着色** — 支持IBL、SSAO、屏幕空间次表面散射、Kajiya-Kay毛发渲染
- **可视化编辑器** — ImGui编辑器，支持场景编辑、实体管理、属性修改、资产管理以及引擎参数预览
- **实体组件系统（ECS）** — 组件化架构，内置 Transform、Renderable、Lighting、CameraController 等
- **场景管理** — 采用场景图管理，支持场景创建、加载、保存、同步与序列化
- **资源管理** — 支持 glTF、FBX 等模型格式

## 运行环境

- **操作系统**: Windows / Linux / macOS
- **编译器**: C++17
  - Windows: Visual Studio 2017+ 或 MinGW
  - Linux: GCC 7+ 或 Clang 5+
  - macOS: Xcode Command Line Tools
- **CMake**: 3.20+
- **OpenGL**: 3.3+
- **Python**: 3.6+（构建脚本）

## 依赖

依赖位于 `libs/` 目录：

- **GLFW** — 窗口与输入
- **GLAD** — OpenGL 加载
- **GLM** — 数学
- **Assimp** — 模型加载
- **spdlog** — 日志
- **ImGui** — 即时模式 GUI
- **stb** — 图像加载
- **EnTT** — 实体组件系统（ECS）

依赖以 submodule 管理，克隆时需一并获取：

```bash
# 方式一：递归克隆
git clone --recursive https://github.com/Furry-Monster/RealmEngine

# 方式二：分步克隆
git clone https://github.com/Furry-Monster/RealmEngine
cd RealmEngine
git submodule init
git submodule update
```

## 快速开始

### 使用构建脚本（推荐）

跨平台 Python 构建脚本：

```bash
# 默认 Debug 构建
python scripts/build.py

# Release 构建
python scripts/build.py -t Release

# 清理后构建
python scripts/build.py -c

# 构建并运行
python scripts/build.py -r

# 指定并行任务数
python scripts/build.py -j 8
```

**平台快捷脚本：**

```bash
# Windows
build.bat

# Linux / macOS
./build.sh
```

构建完成后执行 `bin/RealmEngine`（Windows 为 `bin/RealmEngine.exe`）进入编辑器。

### 手动构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# 运行（Linux/macOS）
../bin/RealmEngine
```

## 构建选项

### 构建类型

- `Debug` — 调试（默认）
- `Release` — 发布优化
- `RelWithDebInfo` — 带调试信息的发布
- `MinSizeRel` — 最小体积

### 构建脚本参数

```bash
python scripts/build.py --help

常用参数：
  -t, --type TYPE        构建类型
  -d, --dir DIR          构建目录（默认 build）
  -g, --generator GEN    CMake 生成器
  -j, --jobs N           并行任务数
  -c, --clean            清理
  -r, --run              构建后运行
  -v, --verbose          详细输出
  --configure            仅配置
  --build                仅构建
  -D VAR=VALUE           传递 CMake 变量
```

### 辅助工具

```bash
# 代码格式化
python scripts/format.py
python scripts/format.py --check

# 代码检查
python scripts/lint.py
python scripts/lint.py --fix

# 清理
python scripts/clean.py
python scripts/clean.py --all

# 测试
python scripts/test.py
```

更多说明见 `scripts/GUIDE.md`。

## 许可证

本项目使用MIT许可证，见项目根目录 [LICENSE](LICENSE)。

## 贡献

欢迎提交 Issue 与 Pull Request。

# RealmEngine 构建脚本

[English](GUIDE.md) | [中文](GUIDE_zh.md)

RealmEngine 的跨平台构建系统，支持 Windows、Linux 与 macOS。

## 概述

构建系统由 `scripts/` 目录下的模块化 Python 脚本组成：

- **build.py** — 主构建脚本，跨平台支持
- **build_config.py** — 配置与工具模块
- **format.py** — 使用 clang-format 进行代码格式化
- **lint.py** — 使用 clang-tidy 进行代码检查
- **clean.py** — 清理构建产物
- **test.py** — 测试运行器

## 环境要求

### 通用
- Python 3.6+
- CMake 3.20+
- 支持 C++17 的编译器

### 平台相关

**Windows:**
- Visual Studio 2017+（MSVC）或 MinGW
- 可选：Ninja 构建系统

**Linux:**
- GCC 7+ 或 Clang 5+
- 可选：Ninja 构建系统
- 可选：clang-format、clang-tidy

**macOS:**
- Xcode Command Line Tools
- 可选：Ninja 构建系统
- 可选：clang-format、clang-tidy

## 快速开始

### 基本构建

```bash
# 默认构建（Debug）
python build.py

# Release 构建
python build.py -t Release

# 构建并运行
python build.py -r

# 清理后构建
python build.py -c
```

### 构建选项

```bash
# 显示所有选项
python build.py --help

# 仅配置（不构建）
python build.py --configure

# 仅构建（跳过配置）
python build.py --build

# 指定构建目录
python build.py -d build-release

# 指定并行任务数
python build.py -j 8

# 构建指定目标
python build.py -T RealmEngine

# 详细输出
python build.py -v

# 传递 CMake 变量
python build.py -D CMAKE_CXX_COMPILER=clang++
```

### 构建类型

- **Debug** — 带符号的调试构建（默认）
- **Release** — 优化发布构建
- **RelWithDebInfo** — 带调试信息的发布构建
- **MinSizeRel** — 最小体积发布构建

## 代码质量工具

### 格式化代码

```bash
# 格式化所有源文件
python scripts/format.py

# 仅检查格式，不修改
python scripts/format.py --check

# 格式化指定目录
python scripts/format.py -d src/core

# 详细输出
python scripts/format.py -v
```

### 代码检查

```bash
# 检查所有源文件
python scripts/lint.py

# 检查并自动修复
python scripts/lint.py --fix

# 检查指定目录
python scripts/lint.py -d src/renderer

# 指定检查规则
python scripts/lint.py -c "modernize-*,readability-*"

# 详细输出
python scripts/lint.py -v
```

**注意：** 代码检查需要 `compile_commands.json`。可通过以下命令生成：
```bash
python build.py --configure
```
仅生成 clangd 补全所需数据（只配置不编译）：
```bash
python scripts/build.py --clangd
```
若希望使用 VS (MSVC) 工具链：在「适用于 VS 的 x64 本机工具命令提示」或「Developer Command Prompt for VS」中执行上述命令即可，脚本会使用 Ninja 并自动选用环境中的 `cl.exe`，生成的 `compile_commands.json` 与 VS 构建一致。

## 清理构建产物

```bash
# 清理构建目录
python scripts/clean.py

# 清理全部（build + bin + cache）
python scripts/clean.py --all

# 清理指定目录
python scripts/clean.py --build
python scripts/clean.py --bin
python scripts/clean.py --cache

# 试运行（仅显示将要删除的内容）
python scripts/clean.py --all --dry-run
```

## 运行测试

```bash
# 运行所有测试
python scripts/test.py

# 运行 Release 构建的测试
python scripts/test.py -t Release

# 详细输出
python scripts/test.py -v
```

## 平台相关说明

### Windows

构建系统会自动检测并使用 Visual Studio。若使用 MinGW 或 Ninja：

```bash
# 使用 MinGW Makefiles
python build.py -g "MinGW Makefiles"

# 使用 Ninja（需 ninja 在 PATH 中）
python build.py -g Ninja
```

### Linux

构建系统优先使用 Ninja，否则回退到 Make：

```bash
# 强制使用 Unix Makefiles
python build.py -g "Unix Makefiles"

# 安装 Ninja（Ubuntu/Debian）
sudo apt install ninja-build

# 安装 clang 工具（Ubuntu/Debian）
sudo apt install clang-format clang-tidy
```

### macOS

与 Linux 类似，优先使用 Ninja：

```bash
# 通过 Homebrew 安装工具
brew install cmake ninja llvm

# 使用 LLVM 的 clang-format 与 clang-tidy
python scripts/format.py --clang-format /usr/local/opt/llvm/bin/clang-format
python scripts/lint.py --clang-tidy /usr/local/opt/llvm/bin/clang-tidy
```

## 常用工作流

### 完整清理构建并运行

```bash
python build.py -c -r
```

### 发布构建

```bash
python build.py -t Release -j 8
```

### 开发流程（含代码质量检查）

```bash
# 格式化代码
python scripts/format.py

# 构建
python build.py

# 检查（需先完成构建）
python scripts/lint.py

# 运行
python build.py -r
```

### CI/CD 流程

```bash
# 检查格式
python scripts/format.py --check

# 构建
python build.py -t Release

# 检查
python scripts/lint.py

# 运行测试
python scripts/test.py -t Release
```

## 高级用法

### 自定义 CMake 选项

```bash
# 启用/禁用功能
python build.py -D BUILD_TESTS=ON -D BUILD_DOCS=OFF

# 使用自定义编译器
python build.py -D CMAKE_CXX_COMPILER=clang++

# 设置自定义安装前缀
python build.py -D CMAKE_INSTALL_PREFIX=/opt/realmengine
```

### 多配置构建

```bash
# Debug 构建
python build.py -d build-debug -t Debug

# Release 构建
python build.py -d build-release -t Release

# 运行指定构建
cd bin
./RealmEngine  # Windows 下为 RealmEngine.exe
```

### 并行构建

```bash
# 使用全部 CPU 核心
python build.py -j $(nproc)  # Linux/macOS
python build.py -j %NUMBER_OF_PROCESSORS%  # Windows

# 限制为 4 核
python build.py -j 4
```

## 故障排除

### 未找到 CMake

**错误：** `CMake not found`

**解决：** 从 https://cmake.org/download/ 安装 CMake

### 未找到编译器

**错误：** `Missing required build tools: g++ or clang++`

**Windows：** 安装 Visual Studio 或 MinGW  
**Linux：** `sudo apt install build-essential`  
**macOS：** `xcode-select --install`

### 未找到 Ninja

**错误：** `Ninja not found, falling back to Unix Makefiles`

**解决：** 此为警告，非错误。安装 Ninja 可加速构建：
- Windows：从 https://ninja-build.org/ 下载
- Linux：`sudo apt install ninja-build`
- macOS：`brew install ninja`

### 未找到 compile_commands.json

**错误：** `compile_commands.json not found`

**解决：** 先配置项目：
```bash
python build.py --configure
```

### 编码问题（Windows）

若在 Windows 上遇到编码错误，脚本会自动处理。若仍有问题，可尝试：

```bash
# 设置 UTF-8 编码
chcp 65001
python build.py
```

## 目录结构

```
RealmEngine/
├── build.py              # 主入口（调用 scripts/build.py）
├── scripts/              # 构建脚本目录
│   ├── build.py          # 主构建脚本
│   ├── build_config.py   # 配置模块
│   ├── format.py         # 代码格式化
│   ├── lint.py           # 代码检查
│   └── test.py           # 测试运行器
├── src/                  # 源码
├── build/                # 构建产物（生成）
├── bin/                  # 输出二进制（生成）
└── CMakeLists.txt        # CMake 配置
```

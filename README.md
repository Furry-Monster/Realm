# Realm 渲染引擎
目前处在摸索开发阶段

## 依赖项
图形API: OpenGL Core (3.3) + GLFW
其余依赖项均在Lib目录下，部分库仅出于未来开发考虑，目前并未使用。

## 可用模块：
1. Resource部分，支持FBX，glTF，Obj等多种模型的加载，基本支持PBR。
2. 初步渲染，渲染架构目前仍然比较混乱，但是对于常规模型的加载，把前向渲染部分的片段着色器换成普通PBR（pbr_simple.frag）是可以支持的。

## 编译运行
由于目前处在开发阶段，对于其他平台的支持基本没有。
我使用的是Kubuntu 24.04,内核版本6.14,KDE Plasma作为桌面进行窗口渲染。
其他Linux系统的机器，直接运行根目录下的脚本就可以了。

## 推荐配置
Mem >= 8G
不需要独立显卡，不要使用HDR显示器，我没调framebuffer.

## 代码格式化和 Lint

项目已配置统一的代码格式化和静态分析工具。

### 快速开始

```bash
# 格式化所有代码
make format

# 检查代码格式
make format-check

# 运行 lint 检查
make lint

# 自动修复 lint 问题
make lint-fix
```

或使用脚本：

```bash
./format.sh      # 格式化代码
./lint.sh        # 运行 lint 检查
./lint-fix.sh    # 自动修复 lint 问题
```

详细说明请参考 [代码风格指南](docs/code_style.md)。

### 前置要求

- `clang-format` (>= 10.0)
- `clang-tidy` (>= 10.0)

安装方法：
```bash
sudo apt-get install clang-format clang-tidy
```

# RealmEngine

[English](README.md) | [中文](README_zh.md)

基于 OpenGL 的现代游戏引擎，集成 PBR 渲染管线、可视化编辑器与 ECS 架构。

![editor](docs/editor.png "editor demo")

![pbr + npr](docs/aqua.png "pbr demo")

![pbr + npr](docs/example.png)

![debug](docs/debug.png)

## 特性

- **PBR 渲染** — Cook-Torrance BRDF，金属度/粗糙度工作流，多 Pass 管线（阴影、GTAO、泛光、次表面散射、后处理）
- **基于图像的光照** — 漫反射辐照度、高光预过滤、BRDF 查找表
- **可视化编辑器** — ImGui 场景编辑器，含视口、层级、属性、资产浏览器、性能分析、撤销/重做、快捷键
- **ECS 架构** — 基于 EnTT，内置 Transform、Renderable、Camera、Lighting、Hierarchy、AudioSource、AudioListener 组件
- **音频系统** — miniaudio 集成；空间音频；监听器管理
- **场景管理** — 场景图、JSON 序列化、资源缓存（glTF / FBX / OBJ / PLY / STL）
- **RHI 抽象层** — 当前 OpenGL 后端，架构预留 Vulkan / D3D12

## 环境要求

- **操作系统**: Windows / Linux / macOS
- **编译器**: C++17（MSVC 2017+、GCC 7+、Clang 5+）
- **CMake**: 3.20+ &ensp;|&ensp; **OpenGL**: 3.3+ &ensp;|&ensp; **Python**: 3.6+（构建脚本）

## 快速开始

```bash
git clone --recursive https://github.com/Furry-Monster/RealmEngine.git
cd RealmEngine

# 构建并运行（Python 脚本）
python scripts/build.py -t Release -j 8 -r

# 或手动 CMake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

构建完成后运行 `bin/RealmEditor`（Windows 为 `bin/RealmEditor.exe`）。

## 项目结构

```
RealmEngine/
├── engine/          # 引擎核心库（RHI、渲染器、ECS、平台层）
├── editor/          # 可视化编辑器（面板、命令、桥接层）
├── sandbox/         # 独立沙盒运行端
├── shaders/         # GLSL 着色器
├── assets/          # 模型、纹理、HDR 环境贴图
├── libs/            # 第三方依赖（git 子模块）
├── scripts/         # 构建、格式化、检查、测试脚本
└── CMakeLists.txt
```

## 依赖

全部以 git 子模块管理，位于 `libs/`：

[GLFW](https://www.glfw.org/) | [GLAD](https://glad.dav1d.de/) | [GLM](https://github.com/g-truc/glm) | [Assimp](https://www.assimp.org/) | [spdlog](https://github.com/gabime/spdlog) | [ImGui](https://github.com/ocornut/imgui) | [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | [stb](https://github.com/nothings/stb) | [EnTT](https://github.com/skypjack/entt) | [miniaudio](https://miniaud.io/) | [nlohmann/json](https://github.com/nlohmann/json)

## 构建选项

```bash
python scripts/build.py --help

# 常用
python scripts/build.py                    # Debug 构建
python scripts/build.py -t Release -j 8   # Release，8 线程
python scripts/build.py -c -r             # 清理、构建、运行
python scripts/build.py --no-editor       # 不构建编辑器
python scripts/build.py --no-sandbox      # 不构建沙盒
python scripts/build.py --tests           # 启用测试

# 代码质量
python scripts/format.py                  # clang-format
python scripts/lint.py                    # clang-tidy
```

完整构建文档见 [`scripts/GUIDE_zh.md`](scripts/GUIDE_zh.md)。

## 许可证

MIT 许可证 — 见 [LICENSE](LICENSE)。

## 贡献

欢迎提交 Issue 与 Pull Request。

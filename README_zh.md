# RealmEngine

[English](README.md) | [中文](README_zh.md)

基于 OpenGL 的现代游戏引擎，集成 PBR 渲染管线、可视化编辑器与 ECS 架构。

![editor](docs/editor.png "editor demo")

![pbr + npr](docs/aqua.png "pbr demo")

![pbr + npr](docs/example.png)

![debug](docs/debug.png)

## 特性

- **PBR 渲染** — Cook-Torrance BRDF，金属度/粗糙度工作流，多 Pass 管线（阴影、GTAO、泛光、次表面散射、SSR、后处理）
- **基于图像的光照** — 漫反射辐照度、高光预过滤、BRDF 查找表
- **可视化编辑器** — ImGui 场景编辑器，含视口、层级、属性、资产浏览器、性能分析、撤销/重做、快捷键
- **ECS 架构** — 基于 EnTT，内置 Transform、Renderable、Camera、Lighting、Hierarchy、AudioSource、AudioListener 组件
- **音频系统** — miniaudio 集成；空间音频；监听器管理
- **场景管理** — 场景图、JSON 序列化、资源缓存（glTF / FBX / OBJ / PLY / STL）
- **RHI 抽象层** — 当前 OpenGL 后端，架构预留 Vulkan / D3D12

### 渲染

引擎支持 **Forward** 与 **Deferred** 两种管线，均采用 Cook-Torrance PBR 与金属度/粗糙度工作流。

| 特性 | 说明 |
|------|------|
| **阴影** | 方向光：4 级联 CSM，PCF/PCSS 软阴影；点光：立方体贴图深度（最多 4 盏）；聚光：2D 深度（最多 4 盏）。支持 Shadow 预览模式调试。 |
| **IBL** | 漫反射辐照度立方体贴图、预滤波高光环境贴图、BRDF 查找表；基于 HDRI 的天空盒 |
| **GTAO** | 屏幕空间环境光遮蔽，多方向射线步进与双边模糊 |
| **SSS** | 次表面散射（BSSRDF 风格包裹漫反射），适用于皮肤/布料；Forward 管线通过材质 subsurface 选项启用 |
| **泛光** | 亮度阈值提取，6 级 Mip 链，可分离高斯模糊 |
| **SSR** | 屏幕空间反射（仅 Deferred）；Hi-Z 加速射线步进；Fresnel 与粗糙度调制；后处理合成 |
| **后处理** | AO 混合、泛光叠加、Reinhard 色调映射、Gamma 校正 |
| **显示模式** | Lit、Albedo、Normals、Metallic、Roughness、Material AO、Emissive、AO、Depth、SSR、Shadow 预览 |

Deferred 管线额外包含 G-Buffer（4 RT + 深度）、Hi-Z、聚簇光源剔除（Compute）、SSR；Forward 管线采用直接 PBR 全光源遍历。管线模式可在项目设置中配置。

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

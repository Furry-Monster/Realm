# RealmEngine 技术文档索引

> 本文档库为 RealmEngine 游戏引擎的完整技术文档，旨在帮助读者快速理解项目架构、核心模块设计和关键技术实现。  
> 文档基于源代码深度分析生成，所有类名、函数名、文件路径均与代码库精确对应。

---

## 文档结构

### 总览文档

| 文档 | 内容 | 建议阅读时间 |
|------|------|-------------|
| [**00-Architecture-Overview.md**](./00-Architecture-Overview.md) | 项目技术架构白皮书：项目概览、技术栈、系统架构图、目录结构、核心模块概述、数据模型、环境部署 | 15 分钟 |

### 模块细分文档

| 编号 | 文档 | 对应源码 | 核心内容 | 建议阅读时间 |
|------|------|---------|---------|-------------|
| 01 | [**Rendering Module**](./01-Rendering-Module.md) | `src/render/` | 五阶段渲染管线、PBR、IBL 预计算、Shadow Mapping、Bloom、HDR/Tonemapping、灯光系统 UBO、材质系统、Framebuffer 设计、模型加载 | 20 分钟 |
| 02 | [**Scene Management**](./02-Scene-Management.md) | `src/gameplay/scene/` | Scene Graph 数据结构、SceneNode 树、SceneManager 生命周期、JSON 序列化/反序列化、加密机制 | 10 分钟 |
| 03 | [**Entity-Component**](./03-Entity-Component.md) | `src/gameplay/` | EC 架构设计、Component 类型系统 (RTTI)、Transform/Renderable/Light 组件详解、CameraController、与渲染层的数据桥接 | 10 分钟 |
| 04 | [**Editor Module**](./04-Editor-Module.md) | `src/editor/` | ImGui Docking 集成、Widget 架构、EditorContext 状态共享、各面板 (MenuBar/Hierarchy/Properties/FileDialog) 实现 | 10 分钟 |
| 05 | [**Resource & Config**](./05-Resource-Config.md) | `src/resource/` | ConfigManager 四类配置结构体、ConfigSerializer JSON 序列化、XOR 加密、AssetManager 预留设计 | 8 分钟 |
| 06 | [**Core Infrastructure**](./06-Core-Infrastructure.md) | `src/` 根目录 | GlobalContext (Service Locator)、Engine (Game Loop)、Window (GLFW 封装)、Input (命令系统)、Logger (spdlog)、工具模块 | 12 分钟 |
| 07 | [**Shader & IBL Pipeline**](./07-Shader-IBL-Pipeline.md) | `shaders/` | 18 个 GLSL 着色器详解、Cook-Torrance BRDF 数学推导、GGX/Fresnel/Geometry 函数、IBL Split-Sum 近似、重要性采样、PCF 阴影、Bloom 高斯模糊 | 20 分钟 |

---

## 阅读建议

### 快速了解项目（30 分钟）

1. 先读 **00-Architecture-Overview** 掌握全貌
2. 再读 **01-Rendering-Module** 理解核心渲染管线
3. 浏览 **07-Shader-IBL-Pipeline** 的面试要点总结

### 面试准备（完整阅读）

建议按以下顺序阅读全部文档：

```
00-Architecture-Overview  →  总体架构
        ↓
01-Rendering-Module       →  渲染核心（最重要）
        ↓
07-Shader-IBL-Pipeline    →  着色器数学（高技术含量）
        ↓
03-Entity-Component       →  EC 架构设计
        ↓
02-Scene-Management       →  场景管理
        ↓
06-Core-Infrastructure    →  基础设施
        ↓
04-Editor-Module          →  编辑器设计
        ↓
05-Resource-Config        →  配置管理
```

### 面试高频话题索引

| 面试话题 | 参考文档 | 关键章节 |
|---------|---------|---------|
| PBR 渲染原理 | 01 + 07 | Cook-Torrance BRDF、GGX、Fresnel |
| IBL 实现 | 01 + 07 | Split-Sum 近似、预计算管线 |
| 阴影技术 | 01 + 07 | Shadow Mapping、PCF、Poisson Disk |
| 后处理效果 | 01 + 07 | Bloom Mip Chain、Tonemapping |
| 架构设计 | 00 + 06 | Service Locator、Game Loop |
| 组件化设计 | 03 | EC vs ECS、类型系统 |
| 场景管理 | 02 | Scene Graph、序列化 |
| 编辑器设计 | 04 | ImGui 集成、Widget 架构 |
| 内存管理 | 03 + 06 | shared_ptr/weak_ptr、RAII |
| OpenGL 知识 | 01 + 07 | FBO、UBO、Mipmap、纹理单元 |

---

## 技术栈速查

| 类别 | 技术 |
|------|------|
| 语言 | C++17 + GLSL |
| 图形 API | OpenGL 3.3 (via GLAD) |
| 窗口/输入 | GLFW |
| 数学 | GLM |
| UI | Dear ImGui (Docking) |
| 模型加载 | Assimp |
| 图像加载 | stb_image |
| 日志 | spdlog |
| 序列化 | nlohmann/json |
| 构建 | CMake 3.20+ / Python 脚本 |

---

## 项目亮点一览

1. **完整的 PBR + IBL 渲染管线** — Cook-Torrance BRDF + Split-Sum IBL + PCF Shadow + Mip Chain Bloom
2. **5 阶段渲染调度** — Shadow → PBR → Skybox → Bloom → Post-Processing
3. **灵活的 EC 架构** — 运行时动态组件增删 + RTTI 类型系统
4. **Scene Graph + Entity Map** — 双重索引兼顾层级遍历和随机访问
5. **ImGui Docking 编辑器** — 专业级面板布局 + 场景编辑 + 属性面板
6. **逻辑/渲染分层** — Gameplay 层与 Render 层解耦，通过同步机制桥接
7. **JSON 场景序列化** — 人类可读的场景持久化 + 可选加密
8. **跨平台构建** — CMake + Python 脚本支持 Linux/macOS

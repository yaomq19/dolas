# Dolas 引擎

Dolas 是一个基于 DirectX 11 的轻量级游戏引擎，使用 C++20 开发。

## 特性

- 🎮 基于 DirectX 11 的现代渲染管线
- 📦 使用 Git Submodule 管理第三方依赖
- 📊 集成 Tracy 性能分析工具
- 🎨 支持 DDS 纹理加载（DirectXTex）
- 🔄 多线程任务系统
- 📝 使用 spdlog 的日志系统
- 🖼️ 集成 ImGui（支持 Docking 和多视口）

## 项目结构

```
dolas/
├── source/          # 源代码文件
│   ├── base/        # 基础工具类
│   ├── core/        # 核心引擎代码
│   ├── manager/     # 各类管理器
│   └── render/      # 渲染系统
├── third_party/     # 第三方库（Git Submodule）
│   ├── imgui/       # ImGui (docking branch)
│   ├── assimp/      # Assimp 3D 模型加载
│   ├── tinyxml2/    # TinyXML2 XML 解析
│   ├── spdlog/      # spdlog 日志库
│   ├── DirectXTex/  # DirectXTex 纹理处理
│   └── tracy/       # Tracy 性能分析
├── content/         # 资源文件
│   ├── shader/      # HLSL 着色器文件
│   ├── texture/     # 纹理资源
│   ├── mesh/        # 网格资源
│   └── material/    # 材质资源
├── shader_compiler/ # 着色器编译工具
├── script/          # 构建和工具脚本
└── build/           # 构建输出目录（自动生成）
```

## 构建要求

- **操作系统**: Windows 10 或更高版本
- **编译器**: Visual Studio 2022 或更高版本（支持 C++20）
- **CMake**: 3.15 或更高版本
- **Git**: 用于克隆仓库和管理子模块
- **Vulkan SDK**: （可选）如果需要 Vulkan 支持

## 快速开始

### 1. 克隆项目（包含子模块）

```bash
# 克隆主仓库和所有子模块
git clone --recursive https://github.com/yaomq19/dolas.git
cd dolas

# 如果已经克隆了主仓库，初始化子模块
git submodule update --init --recursive
```

### 2. 配置 CMake

```bash
# 生成 Visual Studio 项目文件
cmake -B build -G "Visual Studio 17 2022"

# 或者使用提供的脚本
script\cmake_generate.bat
```

### 3. 构建项目

```bash
# 使用 CMake 构建
cmake --build build --config Debug

# 或者使用 Visual Studio 打开生成的解决方案
build\Dolas.sln
```

### 4. 运行

```bash
build\bin\Debug\Dolas.exe
```

### 5. 使用便捷脚本

项目提供了几个便捷的批处理脚本，位于 `script/utility/` 目录下：

#### `generate_project.bat`
配置 CMake 项目。首次构建或修改 CMakeLists.txt 后运行。

```bash
script\utility\generate_project.bat
```

#### `build_project.bat`
构建项目到 `build/` 目录。

```bash
script\utility\build_project.bat
```

#### `quick_start.bat`
运行构建好的 Debug 版本应用程序。

```bash
script\utility\quick_start.bat
```

> **提示**: 所有脚本执行完毕后会等待按键退出，方便查看输出结果。

## 第三方库

本项目通过 Git Submodule 管理以下依赖：

| 库名 | 版本 | 用途 | 许可证 | 仓库 |
|------|------|------|--------|------|
| [ImGui](https://github.com/ocornut/imgui) | master (docking) | 即时模式 GUI | MIT | [yaomq19/imgui](https://github.com/yaomq19/imgui) |
| [Assimp](https://github.com/assimp/assimp) | master | 3D 模型加载 | BSD-3-Clause | [yaomq19/assimp](https://github.com/yaomq19/assimp) |
| [TinyXML2](https://github.com/leethomason/tinyxml2) | master | XML 解析 | Zlib | [yaomq19/tinyxml2](https://github.com/yaomq19/tinyxml2) |
| [spdlog](https://github.com/gabime/spdlog) | v1.x | 快速日志库 | MIT | [yaomq19/spdlog](https://github.com/yaomq19/spdlog) |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | main | DirectX 纹理处理 | MIT | [yaomq19/DirectXTex](https://github.com/yaomq19/DirectXTex) |
| [Tracy](https://github.com/wolfpld/tracy) | master | 性能分析工具 | BSD-3-Clause | [yaomq19/tracy](https://github.com/yaomq19/tracy) |
| [Catch2](https://github.com/catchorg/Catch2) | devel (v3) | 单元测试框架 | BSL-1.0 | [yaomq19/Catch2](https://github.com/yaomq19/Catch2) |

> 所有依赖都位于 `third_party/` 目录下，通过 Git Submodule 管理。详细信息请参阅 [third_party/README.md](third_party/README.md)。

### 更新子模块

```bash
# 更新所有子模块到最新版本
git submodule update --remote

# 更新特定子模块
cd third_party/imgui
git pull origin master
cd ../..
git add third_party/imgui
git commit -m "Update ImGui submodule"
```

## 配置选项

### Tracy 性能分析

项目默认启用 Tracy 性能分析（on-demand 模式）。要禁用它，请修改 `source/CMakeLists.txt`：

```cmake
# 注释掉这一行
# target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE)
```

### spdlog 格式化

项目配置 spdlog 使用 C++20 的 `std::format`。如果需要使用外部 fmt 库，请修改 `source/CMakeLists.txt`。

## 开发文档

更多开发相关信息，请参阅 [Developer.md](Developer.md)。

## 许可证

本项目采用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。

各第三方库遵循其各自的许可证，具体信息请参见上表。

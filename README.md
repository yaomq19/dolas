# Dolas 引擎

Dolas 是一个基于 DirectX 11 的轻量级游戏引擎，使用 C++20 开发。

## 特性

- 🎮 基于 DirectX 11 的现代渲染管线
- 🔧 使用 vcpkg 管理第三方依赖
- 📊 集成 Tracy 性能分析工具
- 🎨 支持 DDS 纹理加载（DirectXTex）
- 🔄 多线程任务系统
- 📝 使用 spdlog 的日志系统

## 项目结构

```
dolas/
├── source/          # 源代码文件
│   ├── base/        # 基础工具类
│   ├── core/        # 核心引擎代码
│   ├── manager/     # 各类管理器
│   └── render/      # 渲染系统
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
- **CMake**: 3.10 或更高版本
- **vcpkg**: 用于管理第三方依赖
- **Vulkan SDK**: （可选）如果需要 Vulkan 支持

## 快速开始

### 1. 安装 vcpkg

如果您还没有安装 vcpkg，请参考 [vcpkg 官方文档](https://github.com/microsoft/vcpkg)。

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

设置环境变量 `VCPKG_ROOT` 指向您的 vcpkg 安装目录。

### 2. 克隆项目

```bash
git clone https://github.com/yaomq19/dolas.git
cd dolas
```

### 3. 配置 CMake 预设

编辑 `CMakeUserPresets.json` 文件，设置您的 vcpkg 路径：

```json
{
    "version": 2,
    "configurePresets": [
      {
        "name": "default",
        "inherits": "vcpkg",
        "environment": {
          "VCPKG_ROOT": "C:\\path\\to\\your\\vcpkg"
        }
      }
    ]
}
```

### 4. 配置和构建

vcpkg 会自动安装所需的依赖项。

```bash
# 配置项目（vcpkg 会自动安装依赖）
cmake --preset default

# 构建项目
cmake --build build --config Debug
```

### 5. 运行

```bash
build\bin\Debug\Dolas.exe
```

### 6. 使用便捷脚本

项目提供了几个便捷的批处理脚本，位于 `script/utility/` 目录下：

#### `generate_project.bat`
配置 CMake 项目（使用 default 预设）。首次构建或修改 CMakeLists.txt 后运行。

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

本项目通过 vcpkg 管理以下依赖：

| 库名 | 版本 | 用途 | 许可证 |
|------|------|------|--------|
| [spdlog](https://github.com/gabime/spdlog) | 1.16.0 | 快速日志库 | MIT |
| [nlohmann-json](https://github.com/nlohmann/json) | 3.12.0 | JSON 解析 | MIT |
| [assimp](https://github.com/assimp/assimp) | 6.0.2 | 3D 模型加载 | BSD-3-Clause |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | 2025-07-10 | DirectX 纹理处理 | MIT |
| [Tracy](https://github.com/wolfpld/tracy) | 0.11.1 | 性能分析工具 | BSD-3-Clause |

> 所有依赖通过 `vcpkg.json` 声明，会在首次配置时自动安装。

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
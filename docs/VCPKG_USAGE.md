# 使用 vcpkg 添加 spdlog 库

本文档说明如何在 Dolas 项目中使用 vcpkg 添加和管理依赖库。

## 步骤概览

### 1. 在 vcpkg.json 中添加依赖

在项目根目录的 `vcpkg.json` 文件中添加库名称：

```json
{
    "dependencies": [
        "directxtex",
        "spdlog"
    ]
}
```

### 2. 在 CMakeLists.txt 中引用库

在 `source/CMakeLists.txt` 中：

**查找包：**
```cmake
find_package(spdlog CONFIG REQUIRED)
```

**链接库：**
```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
```

### 3. 安装依赖并重新配置

在命令行中执行：

```bash
# 方式 1: 如果使用 vcpkg 集成模式
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg根目录]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# 方式 2: 如果已经配置了 vcpkg 环境变量
cd build
cmake ..
cmake --build . --config Release
```

vcpkg 会自动下载、编译并安装 spdlog 库。

### 4. 在代码中使用

```cpp
#include <spdlog/spdlog.h>

// 基本使用
spdlog::info("欢迎使用 spdlog!");
spdlog::error("发生错误: {}", error_message);

// 使用格式化
spdlog::warn("警告: 值 = {}, 状态 = {}", value, status);
```

## 常用 vcpkg 命令

```bash
# 查看已安装的包
vcpkg list

# 搜索包
vcpkg search spdlog

# 查看包信息
vcpkg info spdlog

# 手动安装包（通常不需要，CMake 会自动处理）
vcpkg install spdlog

# 更新所有包
vcpkg upgrade
```

## 添加其他库的步骤

1. 在 `vcpkg.json` 的 `dependencies` 数组中添加库名
2. 在 `source/CMakeLists.txt` 中使用 `find_package(包名 CONFIG REQUIRED)`
3. 使用 `target_link_libraries()` 链接库
4. 重新运行 CMake 配置

## 常用的 C++ 库

```json
{
    "dependencies": [
        "directxtex",      // DirectX 纹理处理
        "spdlog",          // 日志库
        "fmt",             // 格式化库
        "nlohmann-json",   // JSON 解析
        "boost",           // Boost 库
        "glm",             // 数学库
        "imgui",           // GUI 库
        "assimp",          // 3D 模型加载
        "stb",             // 图像加载
        "catch2"           // 单元测试
    ]
}
```

## 注意事项

1. **库名大小写**：vcpkg.json 中的库名通常使用小写（如 `directxtex`）
2. **CMake 包名**：CMakeLists.txt 中的包名可能不同（如 `DirectXTex`），查看库文档
3. **目标名称**：链接时使用 `::` 命名空间（如 `spdlog::spdlog`）
4. **自动安装**：使用 vcpkg manifest 模式，首次 CMake 配置时会自动安装依赖
5. **缓存**：已安装的包会被缓存，重新配置时不会重复下载

## 相关文件

- `vcpkg.json` - vcpkg 依赖清单
- `vcpkg-configuration.json` - vcpkg 配置
- `CMakeLists.txt` - 项目主配置
- `source/CMakeLists.txt` - 源代码配置

## 参考资源

- [vcpkg 官方文档](https://vcpkg.io/)
- [spdlog GitHub](https://github.com/gabime/spdlog)
- [CMake find_package 文档](https://cmake.org/cmake/help/latest/command/find_package.html)


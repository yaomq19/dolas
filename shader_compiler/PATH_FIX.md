# Shader目录路径问题修复

## 问题描述

运行ShaderCompiler时出现警告：
```
Warning: No .hlsl files found in directory: C:/repos/dolas/shader_compiler/../content/shader
```

但实际上content/shader目录下确实存在.hlsl文件。

## 问题原因

1. **相对路径解析问题**: CMakeLists.txt中使用了相对路径`../content/shader`
2. **路径规范化不充分**: 包含`..`的路径在某些文件系统操作中可能出现问题
3. **工作目录依赖**: 程序运行时的工作目录可能影响相对路径解析

## 解决方案

### 1. 修改CMakeLists.txt使用绝对路径

**修改前:**
```cmake
SHADER_CONTENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../content/shader"
```

**修改后:**
```cmake
SHADER_CONTENT_DIR="${CMAKE_SOURCE_DIR}/content/shader"
```

这样可以确保路径总是相对于项目根目录的绝对路径。

### 2. 改进路径规范化

在`main.cpp`中添加路径规范化：
```cpp
// 规范化路径
shader_dir = FileUtils::NormalizePath(shader_dir);
```

### 3. 改进文件扫描函数

在`file_utils.cpp`中改进`GetFilesWithExtension`函数：
```cpp
// 首先规范化目录路径
fs::path dir_path = fs::path(directory).lexically_normal();
```

### 4. 添加调试信息

添加详细的调试日志来跟踪路径解析过程：
```cpp
LOG_DEBUG("Raw shader directory: " + original_path);
LOG_DEBUG("Normalized shader directory: " + normalized_path);
LOG_DEBUG("Directory exists check: " + (exists ? "true" : "false"));
```

## 验证修复

修复后重新编译并运行：

```bash
cd shader_compiler/build
cmake .. 
cmake --build . --config Debug
./bin/Debug/ShaderCompiler.exe
```

应该看到正确的输出：
```
Found 4 .hlsl files:
  deferred_shading.ps.hlsl
  deferred_shading.vs.hlsl
  solid.ps.hlsl
  solid.vs.hlsl
```

## 相关文件

- `shader_compiler/CMakeLists.txt` - 路径宏定义
- `shader_compiler/src/main.cpp` - 路径初始化和规范化
- `shader_compiler/src/file_utils.cpp` - 文件扫描函数
- `shader_compiler/src/shader_compiler.cpp` - 调试信息添加

## 防止类似问题

1. **使用绝对路径**: 在CMake中优先使用`CMAKE_SOURCE_DIR`等绝对路径变量
2. **路径规范化**: 总是对用户输入或配置的路径进行规范化处理
3. **充分的错误处理**: 在文件系统操作中添加详细的错误检查和日志
4. **调试信息**: 在关键路径操作中添加调试日志便于问题排查

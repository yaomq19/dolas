# Dolas Shader Compiler

专为Dolas引擎设计的Shader编译器工具，用于持续监视和验证Shader文件的编译有效性。

## 功能特点

- 🔍 **自动扫描**: 自动扫描`content/shader/`目录下的所有`.hlsl`文件
- 📋 **批量编译**: 支持批量编译或指定文件编译
- 📝 **详细日志**: 输出详细的编译日志到控制台和文件
- ❌ **错误跟踪**: 专门记录编译错误的详细信息
- ⚡ **快速诊断**: 快速识别shader类型和入口点
- 📊 **统计报告**: 提供编译结果统计和性能数据

## 编译和安装

### 前提条件

- Windows 10/11
- Visual Studio 2019 或更新版本
- CMake 3.10+
- Windows SDK（包含D3DCompiler）

### 构建步骤

```bash
# 在项目根目录下
cd shader_compiler
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

编译完成后，可执行文件位于：`build/bin/ShaderCompiler.exe`

## 使用方法

### 基本用法

```bash
# 编译所有shader文件
ShaderCompiler.exe

# 编译指定文件
ShaderCompiler.exe deferred_shading.vs.hlsl solid.ps.hlsl

# 显示帮助
ShaderCompiler.exe --help
```

### 文件命名规范

工具会根据文件名自动识别shader类型：

- `*.vs.hlsl` → Vertex Shader (vs_5_0)
- `*.ps.hlsl` → Pixel Shader (ps_5_0)  
- `*.gs.hlsl` → Geometry Shader (gs_5_0)
- `*.hs.hlsl` → Hull Shader (hs_5_0)
- `*.ds.hlsl` → Domain Shader (ds_5_0)
- `*.cs.hlsl` → Compute Shader (cs_5_0)

默认入口点为`main`函数。

### 日志输出

工具会在以下位置生成日志：

- **控制台输出**: 实时显示编译进度和结果
- **主日志**: `logs/shader_compiler_YYYYMMDD_HHMMSS.log`
- **错误日志**: `logs/compilation_errors.log`

## 输出示例

### 成功编译示例

```
=====================================================
    Dolas Shader Compiler v1.0
    持续监视Shader文件编译有效性工具
=====================================================

Shader搜索目录: content/shader
找到 4 个.hlsl文件:
  deferred_shading.vs.hlsl
  deferred_shading.ps.hlsl
  solid.vs.hlsl
  solid.ps.hlsl

[1/4] 编译: deferred_shading.vs.hlsl... yes (15.23ms)
[2/4] 编译: deferred_shading.ps.hlsl... yes (23.45ms)
[3/4] 编译: solid.vs.hlsl... yes (12.67ms)
[4/4] 编译: solid.ps.hlsl... yes (18.91ms)

=====================================================
               编译结果摘要
=====================================================
成功编译的文件:
  yes deferred_shading.vs.hlsl    [vs_5_0] (15.23ms, 1024 bytes)
  yes deferred_shading.ps.hlsl    [ps_5_0] (23.45ms, 2048 bytes)
  yes solid.vs.hlsl               [vs_5_0] (12.67ms, 896 bytes)
  yes solid.ps.hlsl               [ps_5_0] (18.91ms, 1536 bytes)

编译失败的文件:
  (无)

总计: 4 个文件
成功: 4 个
失败: 0 个
总耗时: 70 ms

所有shader文件编译成功! yes
```

### 错误编译示例

```
[1/2] 编译: broken_shader.ps.hlsl... x

=====================================================
               编译结果摘要
=====================================================
成功编译的文件:
  yes good_shader.vs.hlsl         [vs_5_0] (15.23ms, 1024 bytes)

编译失败的文件:
  x broken_shader.ps.hlsl       [ps_5_0]
    broken_shader.ps.hlsl(10,5): error X3004: undeclared identifier 'wrongVar'
    broken_shader.ps.hlsl(15,12): error X3013: 'main': function does not return a value
    ... (更多错误信息请查看日志文件)

总计: 2 个文件
成功: 1 个
失败: 1 个
总耗时: 45 ms

注意: 检查 logs/compilation_errors.log 查看详细错误信息
```

## 集成到开发流程

### 作为构建步骤

可以将shader编译器集成到CMake构建流程中：

```cmake
# 添加自定义目标
add_custom_target(CompileShaders
    COMMAND ${CMAKE_BINARY_DIR}/shader_compiler/bin/ShaderCompiler.exe
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "编译所有shader文件"
)

# 使主项目依赖shader编译
add_dependencies(Dolas CompileShaders)
```

### 持续集成

在CI/CD管道中使用：

```bash
# 编译shader并检查结果
./ShaderCompiler.exe
if [ $? -ne 0 ]; then
    echo "Shader编译失败，构建中止"
    exit 1
fi
```

### 开发时监控

可以编写脚本定期运行，监控shader文件的有效性：

```bash
# watch_shaders.bat
@echo off
:loop
ShaderCompiler.exe
timeout /t 30 > nul
goto loop
```

## 故障排除

### 常见问题

1. **找不到D3DCompiler.dll**
   - 确保安装了Windows SDK
   - 检查系统PATH是否包含SDK路径

2. **无法读取shader文件**
   - 检查文件权限
   - 确认文件路径正确

3. **包含文件找不到**
   - 确保头文件在`content/shader/`目录下
   - 检查`#include`路径是否正确

### 调试模式

编译时启用调试模式可获得更详细的信息：

```bash
cmake --build . --config Debug
```

## 贡献

欢迎提交问题报告和改进建议！

## 许可证

与Dolas引擎使用相同的许可证。

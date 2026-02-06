# 编译说明

## 修复的编译问题

### 问题: ID3DInclude 不能使用 ComPtr 管理

**错误信息:**
```
error C2039: "Release": 不是 "ID3DInclude" 的成员
```

**原因:**
`ID3DInclude` 不是标准的COM接口，不继承自`IUnknown`，因此不能使用`Microsoft::WRL::ComPtr`来管理。

**解决方案:**
将以下代码：
```cpp
ComPtr<ID3DInclude> include_handler = nullptr;
// ...
include_handler.Get()
```

修改为：
```cpp
ID3DInclude* include_handler = nullptr;
// ...
include_handler
```

## 编译步骤

### 方法1: 使用Visual Studio

1. 打开主项目的 `Dolas.sln`
2. 右键点击 `ShaderCompiler` 项目
3. 选择"生成"

### 方法2: 使用CMake命令行

```bash
cd shader_compiler
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```

### 方法3: 使用MSBuild命令行

```bash
cd build
msbuild ShaderCompiler.vcxproj /p:Configuration=Release /p:Platform=x64
```

## 验证编译结果

编译成功后，可执行文件位于：
- Debug: `build/bin/Debug/ShaderCompiler.exe`
- Release: `build/bin/Release/ShaderCompiler.exe`

运行测试：
```bash
# 在 shader_compiler 目录下
build\bin\Release\ShaderCompiler.exe --help
```

或者运行快速测试脚本：
```bash
test_shader_compiler.bat
```

## 常见编译问题

### 1. 找不到 Windows SDK
确保安装了 Windows 10/11 SDK，包含 D3DCompiler 组件。

### 2. CMake 版本过低
需要 CMake 3.10 或更高版本。

### 3. C++ 标准支持
需要支持 C++17 的编译器（Visual Studio 2017 或更新版本）。

## 依赖项

- **D3DCompiler**: Windows SDK 提供
- **std::filesystem**: C++17 标准库
- **Windows API**: 用于文件系统操作

## 输出文件

编译成功后会生成：
- `ShaderCompiler.exe`: 主可执行文件
- 日志文件将在运行时创建在 `logs/` 目录下

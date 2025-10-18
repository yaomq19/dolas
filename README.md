# Dolas 引擎

Dolas 是一个基于 DirectX 11 的轻量级游戏引擎。

## 项目结构

- `source/` - 源代码文件
- `shaders/` - HLSL 着色器文件
- `third_party/` - 第三方依赖库
- `content/` - 资源文件
- `config/` - 配置文件
- `scripts/` - 构建和工具脚本

## 构建要求

- Windows 10或更高版本
- Visual Studio 2019或更高版本
- CMake 3.10或更高版本
- DirectX SDK

## 构建步骤

1. 克隆仓库
```
git clone https://github.com/yourusername/dolas.git
cd dolas
```

2. 创建构建目录
```
mkdir build
cd build
```

3. 配置CMake
```
cmake ..
```

4. 构建项目
```
cmake --build . --config Release
```

5. 运行
```
bin\Release\Dolas.exe
```

## 许可证

本项目采用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。

## 第三方库

本项目使用了以下第三方库：
- **Assimp** - 3D模型加载库
- **DirectXTex** - DirectX纹理处理库
- **nlohmann/json** - JSON解析库
- **spdlog** - 日志库
- **Optick** - 性能分析工具
- **ThreadPool** - 线程池实现

各第三方库遵循其各自的许可证。 
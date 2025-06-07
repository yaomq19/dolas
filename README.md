# Dolas 引擎

Dolas 是一个基于 DirectX 11 的轻量级渲染引擎。

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

[添加适当的许可证信息] 
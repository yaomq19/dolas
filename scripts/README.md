# Dolas Engine - 着色器编译脚本

这个目录包含用于自动编译HLSL着色器的脚本和工具。

## 文件说明

### 着色器编译
- `CompileShaders.py` - 主要的Python编译脚本
- `compile_shaders.bat` - Windows批处理文件，方便运行

### DDS纹理生成
- `DDSGenerate.py` - DDS纹理生成器
- `generate_dds.bat` - DDS生成批处理文件

### 文档
- `README.md` - 本说明文档

# 着色器编译系统

## 功能特性

### ✨ 自动发现
- 递归搜索 `shaders/` 目录下所有 `.hlsl` 文件
- 自动检测入口点函数（VS_Entry*, PS_Entry*, CS_Entry*）

### 📂 目录结构保持
- 编译后的 `.cso` 文件保持与源码相同的目录层次
- 输出到 `build/bin/shaders/` 目录

### 🎯 多入口点支持
- 一个HLSL文件可包含多个着色器入口点
- 每个入口点编译为独立的 `.cso` 文件

### 📝 文件命名规则
```
源文件: example.hlsl
入口点: VS_Entry_Basic, PS_Entry_Solid
输出:   example_VS_Entry_Basic.cso
        example_PS_Entry_Solid.cso
```

## 使用方法

### 方法1：Windows批处理（推荐）
```bash
# 基本编译
scripts\compile_shaders.bat

# 详细输出
scripts\compile_shaders.bat --verbose
```

### 方法2：直接运行Python脚本
```bash
# 基本用法
python scripts/CompileShaders.py

# 指定输出目录
python scripts/CompileShaders.py --output-dir custom/output/path

# 详细输出
python scripts/CompileShaders.py --verbose

# 帮助信息
python scripts/CompileShaders.py --help
```

### 方法3：CMake集成
在CMakeLists.txt中添加自定义目标：
```cmake
add_custom_target(CompileShaders
    COMMAND python scripts/CompileShaders.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compiling HLSL shaders..."
)
```

## 入口点命名规范

着色器函数必须按以下规范命名：

### 顶点着色器
```hlsl
void VS_Entry_Basic(VS_INPUT input, out PS_INPUT output) { ... }
void VS_Entry_Advanced(VS_INPUT input, out PS_INPUT output) { ... }
```
编译为: `vs_5_0` 配置文件

### 像素着色器
```hlsl
void PS_Entry_Solid(PS_INPUT input, out float4 color : SV_TARGET) { ... }
void PS_Entry_Textured(PS_INPUT input, out float4 color : SV_TARGET) { ... }
```
编译为: `ps_5_0` 配置文件

### 计算着色器
```hlsl
[numthreads(8, 8, 1)]
void CS_Entry_Generate(uint3 id : SV_DispatchThreadID) { ... }
```
编译为: `cs_5_0` 配置文件

## 目录结构示例

### 编译前
```
project/
├── shaders/
│   ├── basic.hlsl
│   ├── lighting/
│   │   └── phong.hlsl
│   └── postprocess/
│       ├── blur.hlsl
│       └── tonemap.hlsl
└── scripts/
    └── CompileShaders.py
```

### 编译后
```
project/
├── build/bin/shaders/
│   ├── basic_VS_Entry_Main.cso
│   ├── basic_PS_Entry_Main.cso
│   ├── lighting/
│   │   ├── phong_VS_Entry_Phong.cso
│   │   └── phong_PS_Entry_Phong.cso
│   └── postprocess/
│       ├── blur_CS_Entry_Horizontal.cso
│       ├── blur_CS_Entry_Vertical.cso
│       └── tonemap_PS_Entry_ACES.cso
└── ...
```

---

# DDS纹理生成系统

## 功能特性

### 🖼️ 多格式支持
- 支持输入格式：`.jpg`, `.jpeg`, `.png`, `.bmp`, `.tga`, `.tiff`
- 输出统一为 `.dds` 格式

### 🔄 自动Mipmap生成
- 使用BC3_UNORM（DXT5）压缩格式
- 自动生成完整的mipmap链
- 适合游戏引擎纹理使用

### 📂 递归处理
- 递归扫描 `content/textures/` 目录
- 保持原有目录结构
- 同名文件转换（仅扩展名改变）

### ⚡ 智能更新
- 增量更新：仅转换修改过的文件
- 文件时间戳检查
- 可强制重新生成所有文件

## 使用方法

### 方法1：Windows批处理（推荐）
```bash
# 基本转换
scripts\generate_dds.bat

# 强制重新生成所有文件
scripts\generate_dds.bat --force

# 清理所有DDS文件
scripts\generate_dds.bat --clean

# 显示使用信息
scripts\generate_dds.bat --info
```

### 方法2：直接运行Python脚本
```bash
# 基本用法
python scripts/DDSGenerate.py

# 强制重新生成
python scripts/DDSGenerate.py --force

# 清理DDS文件
python scripts/DDSGenerate.py --clean

# 显示帮助
python scripts/DDSGenerate.py --help
```

## 工具要求

### 必需工具：texconv.exe
DDS生成器使用Microsoft的 `texconv.exe` 工具进行纹理转换。

#### 获取方式：
1. **从GitHub下载**：
   - 访问 [DirectXTex Releases](https://github.com/Microsoft/DirectXTex/releases)
   - 下载最新版本的texconv.exe

2. **安装位置**（选择其一）：
   - `项目根目录/tools/texconv.exe`
   - `scripts/texconv.exe`
   - 添加到系统PATH环境变量

3. **Windows SDK**：
   - 如果安装了Windows 10 SDK，通常已包含texconv.exe
   - 路径：`C:\Program Files (x86)\Windows Kits\10\bin\x64\texconv.exe`

## 输出示例

运行脚本后的典型输出：
```
DDS Texture Generator - Dolas Engine
========================================
Running DDSGenerate.py...
✓ Found texconv.exe: C:\Program Files (x86)\Windows Kits\10\bin\x64\texconv.exe
🔍 Scanning directory: C:\repos\dolas\content\textures
📁 Found 3 image files

🚀 Starting conversion of 3 files...
==================================================
[1/3] 🔄 Converting: hero_albedo.png -> hero_albedo.dds
✅ Successfully converted: hero_albedo.dds
[2/3] ⏭️  Skip stone_normal.jpg (DDS file is up to date)
[3/3] 🔄 Converting: grass_diffuse.bmp -> grass_diffuse.dds
✅ Successfully converted: grass_diffuse.dds
==================================================
🎉 Conversion completed: 3/3 files successful

✅ DDS texture generation successful!
```

## 目录结构示例

### 转换前
```
content/textures/
├── hero_albedo.png
├── stone_normal.jpg
└── grass_diffuse.bmp
```

### 转换后
```
content/textures/
├── hero_albedo.png
├── hero_albedo.dds      ← Generated
├── stone_normal.jpg
├── stone_normal.dds     ← Generated
├── grass_diffuse.bmp
└── grass_diffuse.dds    ← Generated
```

---

## 扩展功能

如需添加新的着色器类型，修改 `CompileShaders.py` 中的 `shader_types` 字典：

```python
self.shader_types = {
    'VS_Entry': {'profile': 'vs_5_0', 'description': 'Vertex Shader'},
    'PS_Entry': {'profile': 'ps_5_0', 'description': 'Pixel Shader'},
    'CS_Entry': {'profile': 'cs_5_0', 'description': 'Compute Shader'},
    'GS_Entry': {'profile': 'gs_5_0', 'description': 'Geometry Shader'},  # 新增
}
``` 
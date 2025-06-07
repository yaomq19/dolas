# Content 资源目录

这个目录包含Dolas引擎的所有游戏资源文件。

## 目录结构

```
content/
├── textures/        # 纹理贴图文件
│   ├── characters/  # 角色纹理
│   ├── environment/ # 环境纹理  
│   ├── ui/          # UI纹理
│   └── effects/     # 特效纹理
├── models/          # 3D模型文件
├── audio/           # 音频文件
├── animations/      # 动画文件
└── data/           # 游戏数据文件
```

## 纹理资源 (textures/)

### 支持的输入格式
- `.jpg` / `.jpeg` - JPEG图片
- `.png` - PNG图片（支持透明通道）
- `.bmp` - 位图文件
- `.tga` - Targa图片
- `.tiff` - TIFF图片

### 自动DDS转换
所有纹理文件会被自动转换为DDS格式，包含完整的mipmap链：

```bash
# 运行DDS转换器
scripts\generate_dds.bat

# 强制重新生成所有DDS文件
scripts\generate_dds.bat --force
```

### 纹理命名规范
建议使用描述性的文件名：
- `hero_albedo.png` - 角色反照率贴图
- `hero_normal.png` - 角色法线贴图
- `hero_roughness.png` - 角色粗糙度贴图
- `grass_diffuse.jpg` - 草地漫反射贴图
- `stone_normal.tga` - 石头法线贴图

### 目录组织
按功能和用途组织纹理：
```
textures/
├── characters/
│   ├── hero/
│   │   ├── hero_albedo.png
│   │   ├── hero_normal.png
│   │   └── hero_roughness.png
│   └── enemy/
│       └── orc_albedo.jpg
├── environment/
│   ├── terrain/
│   │   ├── grass.png
│   │   └── stone.jpg
│   └── sky/
│       └── skybox.png
└── ui/
    ├── buttons/
    │   └── button_normal.png
    └── icons/
        └── health_icon.png
```

## 在C++代码中使用

使用Dolas路径管理系统加载纹理：

```cpp
#include "base/dolas_paths.h"

// 加载DDS纹理
std::wstring texturePath = Dolas::Paths::BuildContentPathW("textures/characters/hero_albedo.dds");
ID3D11ShaderResourceView* textureView;
DirectX::CreateDDSTextureFromFile(device, texturePath.c_str(), nullptr, &textureView);
```

## 注意事项

1. **DDS文件**：自动生成的 `.dds` 文件不要手动编辑或删除
2. **版本控制**：建议将原始图片文件（.png, .jpg等）加入版本控制，而将 `.dds` 文件添加到 `.gitignore`
3. **文件大小**：注意纹理文件大小，大纹理会显著影响加载时间和内存使用
4. **格式选择**：
   - 使用 `.png` 保存需要透明通道的纹理
   - 使用 `.jpg` 保存不需要透明通道的纹理（文件更小）
   - 使用 `.tga` 保存需要高质量的纹理

## 性能建议

- **纹理尺寸**：使用2的幂次方尺寸（256x256, 512x512, 1024x1024等）
- **压缩格式**：DDS转换器会自动使用BC3_UNORM压缩，提供良好的质量和性能平衡
- **Mipmap**：自动生成的mipmap可以改善远距离渲染性能和质量 
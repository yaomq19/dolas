#ifndef DOLAS_TEXTURE_MANAGER_H
#define DOLAS_TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_texture.h"
#include "base/dolas_base.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    enum class TextureUsage {
        Immutable,      // 只读纹理 (CPU不可访问，GPU只读)
        Dynamic,        // 动态纹理 (CPU频繁写，GPU读)
        RenderTarget,   // 渲染目标 (GPU读写)
        DepthStencil,   // 深度模板缓冲区
        Staging         // 数据中转 (CPU读写)
    };
    
    struct TextureDescriptor {
        uint32_t width;             // 纹理宽度
        uint32_t height;            // 纹理高度
        TextureFormat format;         // 纹理格式
        TextureUsage usage;           // 核心纹理类型
        bool generateMips = false;  // 是否生成mipmaps
        bool shaderResource = true; // 是否绑定为着色器资源
        uint32_t arraySize = 1;     // 纹理数组大小
        uint32_t sampleCount = 1;   // 多重采样数量
    };

    class TextureManager
    {
    public:
        TextureManager();
        ~TextureManager();
        bool Initialize();
        bool Clear();

        TextureID CreateTextureFromFile(const std::string& file_name);

        TextureID CreateTexture2D(
            ID3D11Device* device, 
            const TextureDescriptor& desc,
            const D3D11_SUBRESOURCE_DATA* initData = nullptr);

        TextureID CreateTexture(
            TextureType texture_type,
            TextureFormat texture_format,
            uint32_t width,
            uint32_t height,
            uint32_t mip_levels,
            uint32_t sample_count,
            uint32_t sample_quality,
            uint32_t array_size,
            uint32_t bind_flags,
            uint32_t cpu_access_flags,
            uint32_t misc_flags,
            D3D11_USAGE usage
        );
        Texture* GetTexture(TextureID texture_id);

    private:
        DXGI_FORMAT ConvertToDXGIFormat(TextureFormat format);
        Bool IsDepthFormatShaderCompatible(DXGI_FORMAT format);
        std::unordered_map<TextureID, Texture*> m_textures;
    }; // class TextureManager
} // namespace Dolas

#endif // DOLAS_TEXTURE_MANAGER_H

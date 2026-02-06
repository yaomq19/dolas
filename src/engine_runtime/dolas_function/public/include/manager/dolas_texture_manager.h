#ifndef DOLAS_TEXTURE_MANAGER_H
#define DOLAS_TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_texture.h"
#include "dolas_base.h"
#include "dolas_hash.h"
namespace Dolas
{
    enum class DolasTextureUsage {
        Immutable,      // 只读纹理 (CPU不可访问，GPU只读)
        Dynamic,        // 动态纹理 (CPU频繁写，GPU读)
        RenderTarget,   // 渲染目标 (GPU读写)
        DepthStencil,   // 深度模板缓冲区
        Staging         // 数据中转 (CPU读写)
    };
    
    struct DolasTexture2DDesc {
        TextureID texture_handle;
        uint32_t width;             // 纹理宽度
        uint32_t height;            // 纹理高度
        DolasTextureFormat format;         // 纹理格式
        DolasTextureUsage usage;           // 核心纹理类型
        bool generateMips = false;  // 是否生成mipmaps
        bool shaderResource = true; // 是否绑定为着色器资源
        uint32_t arraySize = 1;     // 纹理数组大小
        uint32_t sampleCount = 1;   // 多重采样数量
    };

    enum class GlobalTextureType
    {
        GLOBAL_TEXTURE_SKY_BOX,
        GLOBAL_TEXTURE_COUNT
    };

    class TextureManager
    {
    public:
        TextureManager();
        ~TextureManager();
        bool Initialize();
        bool Clear();

		// 根据纹理ID获取纹理
		// texture_id: 纹理ID
		// 返回: 指向纹理的指针，如果未找到则返回nullptr
        Texture* GetTextureByTextureID(TextureID texture_id);

		// 从文件创建纹理
		// file_name: 纹理文件名
		// 返回: 纹理ID，如果创建失败则返回 TEXTURE_ID_EMPTY
        TextureID CreateTextureFromDDSFile(const std::string& file_name);

        TextureID CreateTextureFromHDRFile(const std::string& file_name);

		TextureID CreateTextureFromPNGFile(const std::string& file_name);

        Texture* GetGlobalTexture(GlobalTextureType global_texture_type);

		// 创建2D纹理
		// dolas_texture2d_desc: 2D纹理描述
		// 返回: 是否成功创建纹理
        Bool DolasCreateTexture2D(const DolasTexture2DDesc& dolas_texture2d_desc);
    protected:

        DXGI_FORMAT ConvertToDXGIFormat(DolasTextureFormat format);

        DolasTextureFormat ConvertToTextureFormat(DXGI_FORMAT dxgi_format);

        Bool D3DCreateTexture2D(
            TextureID texture_handle,
            const D3D11_TEXTURE2D_DESC* pDesc);
        
        Bool IsDepthFormatShaderCompatible(DXGI_FORMAT format);

        std::unordered_map<TextureID, Texture*> m_textures;

		std::unordered_map<GlobalTextureType, TextureID> m_global_textures;
    }; // class TextureManager
} // namespace Dolas

#endif // DOLAS_TEXTURE_MANAGER_H

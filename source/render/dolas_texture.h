#ifndef DOLAS_TEXTURE_H
#define DOLAS_TEXTURE_H

#include <string>
#include <d3d11.h>
#include "common/dolas_hash.h"
namespace Dolas
{
    enum class DolasTextureType
    {
        TEXTURE_2D,
        TEXTURE_CUBE,
        TEXTURE_3D,
        TEXTURE_ARRAY_2D
    };

    enum class DolasTextureFormat
    {
        R8G8B8A8_UNORM,
        R8G8B8A8_SRGB,
        BC1_UNORM,
        BC1_SRGB,
        BC3_UNORM,
        BC3_SRGB,
        BC7_UNORM,
        BC7_SRGB,
        R32G32B32A32_FLOAT,
        R16G16B16A16_FLOAT,
        // Depth/Stencil related
        D24_UNORM_S8_UINT,
        D32_FLOAT,
        // Typeless base formats for depth resources (for DSV/SRV creation)
        R32_TYPELESS,
        R24G8_TYPELESS,
        R16_TYPELESS,
        R32G8X24_TYPELESS
    };

    class Texture
    {
        friend class TextureManager;
    public:
        Texture();
        ~Texture();

        void Release();

        // Getters
        ID3D11Texture2D* GetD3DTexture2D() const { return m_d3d_texture_2d; }
        ID3D11ShaderResourceView* GetShaderResourceView();
        
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        uint32_t GetMipLevels() const { return m_mip_levels; }
        DolasTextureType GetTextureType() const { return m_texture_type; }
        DolasTextureFormat GetTextureFormat() const { return m_texture_format; }

        bool m_is_from_file = false;
        TextureID m_file_id;
        DolasTextureType m_texture_type;
        DolasTextureFormat m_texture_format;

    private:

        ID3D11Texture2D* m_d3d_texture_2d = nullptr;
        ID3D11ShaderResourceView* m_d3d_shader_resource_view = nullptr;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_mip_levels = 1;
    }; // class Texture
} // namespace Dolas

#endif // DOLAS_TEXTURE_H

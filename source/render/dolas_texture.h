#ifndef DOLAS_TEXTURE_H
#define DOLAS_TEXTURE_H

#include <string>
#include <d3d11.h>
#include "common/dolas_hash.h"
namespace Dolas
{
    enum class TextureType
    {
        TEXTURE_2D,
        TEXTURE_CUBE,
        TEXTURE_3D,
        TEXTURE_ARRAY_2D
    };

    enum class TextureFormat
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
        D24_UNORM_S8_UINT,
        D32_FLOAT
    };

    class Texture
    {
        friend class TextureManager;
    public:
        Texture();
        ~Texture();

        /*bool LoadFromFile(const std::string& file_path, bool generate_mips = true);
        bool LoadFromMemory(const void* data, size_t size, bool generate_mips = true);*/
        bool CreateTexture(uint32_t width, uint32_t height, TextureFormat format, const void* initial_data = nullptr);
        bool CreateRenderTarget(uint32_t width, uint32_t height, TextureFormat format);
        bool CreateDepthStencil(uint32_t width, uint32_t height);
        void Release();

        // Getters
        ID3D11Texture2D* GetTexture2D() const { return m_texture_2d; }
        ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shader_resource_view; }
        ID3D11RenderTargetView* GetRenderTargetView() const { return m_render_target_view; }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_depth_stencil_view; }
        ID3D11UnorderedAccessView* GetUnorderedAccessView() const { return m_unordered_access_view; }
        
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        uint32_t GetMipLevels() const { return m_mip_levels; }
        TextureType GetTextureType() const { return m_texture_type; }
        TextureFormat GetTextureFormat() const { return m_texture_format; }

        TextureID m_file_id;
        TextureType m_texture_type;
        TextureFormat m_texture_format;

    private:
        DXGI_FORMAT ConvertToDXGIFormat(TextureFormat format);
        /*bool CreateTextureFromImage(const DirectX::ScratchImage& image, bool generate_mips);*/

        ID3D11Texture2D* m_texture_2d = nullptr;
        ID3D11ShaderResourceView* m_shader_resource_view = nullptr;
        ID3D11RenderTargetView* m_render_target_view = nullptr;
        ID3D11DepthStencilView* m_depth_stencil_view = nullptr;
        ID3D11UnorderedAccessView* m_unordered_access_view = nullptr;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_mip_levels = 1;
    }; // class Texture
} // namespace Dolas

#endif // DOLAS_TEXTURE_H

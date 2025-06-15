#include "render/dolas_texture.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include <iostream>
// #include <DirectXTex.h>

namespace Dolas
{
    Texture::Texture()
    {
        m_texture_type = TextureType::TEXTURE_2D;
        m_texture_format = TextureFormat::R8G8B8A8_UNORM;
    }

    Texture::~Texture()
    {
        Release();
    }

    //bool Texture::LoadFromFile(const std::string& file_path, bool generate_mips)
    //{
    //    //// 使用DirectXTex加载纹理
    //    //DirectX::ScratchImage image;
    //    //std::wstring wide_path(file_path.begin(), file_path.end());
    //    //
    //    //HRESULT hr = DirectX::LoadFromWICFile(wide_path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
    //    //if (FAILED(hr))
    //    //{
    //    //    // 尝试加载DDS格式
    //    //    hr = DirectX::LoadFromDDSFile(wide_path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
    //    //    if (FAILED(hr))
    //    //    {
    //    //        // 尝试加载TGA格式
    //    //        hr = DirectX::LoadFromTGAFile(wide_path.c_str(), nullptr, image);
    //    //        if (FAILED(hr))
    //    //        {
    //    //            std::cerr << "Texture::LoadFromFile: Failed to load texture from " << file_path << std::endl;
    //    //            return false;
    //    //        }
    //    //    }
    //    //}

    //    //m_file_path = file_path;
    //    //return CreateTextureFromImage(image, generate_mips);
    //}

    //bool Texture::LoadFromMemory(const void* data, size_t size, bool generate_mips)
    //{
    //    //DirectX::ScratchImage image;
    //    //
    //    //// 尝试不同格式的加载
    //    //HRESULT hr = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS_NONE, nullptr, image);
    //    //if (FAILED(hr))
    //    //{
    //    //    hr = DirectX::LoadFromDDSMemory(data, size, DirectX::DDS_FLAGS_NONE, nullptr, image);
    //    //    if (FAILED(hr))
    //    //    {
    //    //        hr = DirectX::LoadFromTGAMemory(data, size, nullptr, image);
    //    //        if (FAILED(hr))
    //    //        {
    //    //            std::cerr << "Texture::LoadFromMemory: Failed to load texture from memory" << std::endl;
    //    //            return false;
    //    //        }
    //    //    }
    //    //}

    //    //return CreateTextureFromImage(image, generate_mips);
    //}

    bool Texture::CreateTexture(uint32_t width, uint32_t height, TextureFormat format, const void* initial_data)
    {
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
        if (!device)
        {
            std::cerr << "Texture::CreateTexture: D3D11 device is null" << std::endl;
            return false;
        }

        m_width = width;
        m_height = height;
        m_texture_format = format;
        m_mip_levels = 1;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = ConvertToDXGIFormat(format);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA init_data = {};
        D3D11_SUBRESOURCE_DATA* p_init_data = nullptr;
        
        if (initial_data)
        {
            init_data.pSysMem = initial_data;
            init_data.SysMemPitch = width * 4; // 假设RGBA格式
            init_data.SysMemSlicePitch = 0;
            p_init_data = &init_data;
        }

        HRESULT hr = device->CreateTexture2D(&desc, p_init_data, &m_texture_2d);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateTexture: Failed to create texture2D" << std::endl;
            return false;
        }

        // 创建Shader Resource View
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = desc.MipLevels;
        srv_desc.Texture2D.MostDetailedMip = 0;

        hr = device->CreateShaderResourceView(m_texture_2d, &srv_desc, &m_shader_resource_view);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateTexture: Failed to create shader resource view" << std::endl;
            return false;
        }

        std::cout << "Texture::CreateTexture: Successfully created " << width << "x" << height << " texture" << std::endl;
        return true;
    }

    bool Texture::CreateRenderTarget(uint32_t width, uint32_t height, TextureFormat format)
    {
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
        if (!device)
        {
            std::cerr << "Texture::CreateRenderTarget: D3D11 device is null" << std::endl;
            return false;
        }

        m_width = width;
        m_height = height;
        m_texture_format = format;
        m_mip_levels = 1;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = ConvertToDXGIFormat(format);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_texture_2d);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateRenderTarget: Failed to create texture2D" << std::endl;
            return false;
        }

        // 创建Render Target View
        hr = device->CreateRenderTargetView(m_texture_2d, nullptr, &m_render_target_view);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateRenderTarget: Failed to create render target view" << std::endl;
            return false;
        }

        // 创建Shader Resource View
        hr = device->CreateShaderResourceView(m_texture_2d, nullptr, &m_shader_resource_view);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateRenderTarget: Failed to create shader resource view" << std::endl;
            return false;
        }

        std::cout << "Texture::CreateRenderTarget: Successfully created " << width << "x" << height << " render target" << std::endl;
        return true;
    }

    bool Texture::CreateDepthStencil(uint32_t width, uint32_t height)
    {
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
        if (!device)
        {
            std::cerr << "Texture::CreateDepthStencil: D3D11 device is null" << std::endl;
            return false;
        }

        m_width = width;
        m_height = height;
        m_texture_format = TextureFormat::D24_UNORM_S8_UINT;
        m_mip_levels = 1;

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT hr = device->CreateTexture2D(&desc, nullptr, &m_texture_2d);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateDepthStencil: Failed to create texture2D" << std::endl;
            return false;
        }

        // 创建Depth Stencil View
        hr = device->CreateDepthStencilView(m_texture_2d, nullptr, &m_depth_stencil_view);
        if (FAILED(hr))
        {
            std::cerr << "Texture::CreateDepthStencil: Failed to create depth stencil view" << std::endl;
            return false;
        }

        std::cout << "Texture::CreateDepthStencil: Successfully created " << width << "x" << height << " depth stencil" << std::endl;
        return true;
    }

    //bool Texture::CreateTextureFromImage(const DirectX::ScratchImage& image, bool generate_mips)
    //{
    //    ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
    //    if (!device)
    //    {
    //        std::cerr << "Texture::CreateTextureFromImage: D3D11 device is null" << std::endl;
    //        return false;
    //    }

    //    const DirectX::TexMetadata& metadata = image.GetMetadata();
    //    m_width = static_cast<uint32_t>(metadata.width);
    //    m_height = static_cast<uint32_t>(metadata.height);
    //    m_mip_levels = static_cast<uint32_t>(metadata.mipLevels);

    //    DirectX::ScratchImage mip_chain;
    //    const DirectX::ScratchImage* final_image = &image;

    //    // 生成mipmap
    //    if (generate_mips && metadata.mipLevels == 1)
    //    {
    //        HRESULT hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, DirectX::TEX_FILTER_DEFAULT, 0, mip_chain);
    //        if (SUCCEEDED(hr))
    //        {
    //            final_image = &mip_chain;
    //            m_mip_levels = static_cast<uint32_t>(mip_chain.GetMetadata().mipLevels);
    //        }
    //    }

    //    // 创建纹理
    //    HRESULT hr = DirectX::CreateTexture(device, final_image->GetImages(), final_image->GetImageCount(), final_image->GetMetadata(), reinterpret_cast<ID3D11Resource**>(&m_texture_2d));
    //    if (FAILED(hr))
    //    {
    //        std::cerr << "Texture::CreateTextureFromImage: Failed to create texture from image" << std::endl;
    //        return false;
    //    }

    //    // 创建Shader Resource View
    //    hr = DirectX::CreateShaderResourceView(device, final_image->GetImages(), final_image->GetImageCount(), final_image->GetMetadata(), &m_shader_resource_view);
    //    if (FAILED(hr))
    //    {
    //        std::cerr << "Texture::CreateTextureFromImage: Failed to create shader resource view" << std::endl;
    //        return false;
    //    }

    //    std::cout << "Texture::CreateTextureFromImage: Successfully created texture " << m_width << "x" << m_height << " with " << m_mip_levels << " mip levels" << std::endl;
    //    return true;
    //}

    DXGI_FORMAT Texture::ConvertToDXGIFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8G8B8A8_UNORM:     return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::R8G8B8A8_SRGB:      return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case TextureFormat::BC1_UNORM:          return DXGI_FORMAT_BC1_UNORM;
            case TextureFormat::BC1_SRGB:           return DXGI_FORMAT_BC1_UNORM_SRGB;
            case TextureFormat::BC3_UNORM:          return DXGI_FORMAT_BC3_UNORM;
            case TextureFormat::BC3_SRGB:           return DXGI_FORMAT_BC3_UNORM_SRGB;
            case TextureFormat::BC7_UNORM:          return DXGI_FORMAT_BC7_UNORM;
            case TextureFormat::BC7_SRGB:           return DXGI_FORMAT_BC7_UNORM_SRGB;
            case TextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::R16G16B16A16_FLOAT:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::D24_UNORM_S8_UINT:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::D32_FLOAT:          return DXGI_FORMAT_D32_FLOAT;
            default:                                 return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    }

    void Texture::Release()
    {
        if (m_shader_resource_view)  { m_shader_resource_view->Release();  m_shader_resource_view = nullptr; }
        if (m_render_target_view)    { m_render_target_view->Release();    m_render_target_view = nullptr; }
        if (m_depth_stencil_view)    { m_depth_stencil_view->Release();    m_depth_stencil_view = nullptr; }
        if (m_unordered_access_view) { m_unordered_access_view->Release(); m_unordered_access_view = nullptr; }
        if (m_texture_2d)            { m_texture_2d->Release();            m_texture_2d = nullptr; }

        m_width = 0;
        m_height = 0;
        m_mip_levels = 1;
    }

} // namespace Dolas

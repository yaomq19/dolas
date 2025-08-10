
#include <iostream>
#include <fstream>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_texture_manager.h"
#include "render/dolas_texture.h"
#include "base/dolas_base.h"
#include "base/dolas_string_util.h"
#include "base/dolas_paths.h"
#include "DirectXTex/DDSTextureLoader11.h"

namespace Dolas
{
    TextureManager::TextureManager()
    {

    }

    TextureManager::~TextureManager()
    {

    }
    
    bool TextureManager::Initialize()
    {
        return true;
    }

    bool TextureManager::Clear()
    {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it)
        {
            Texture* texture = it->second;
            if (texture)
            {
                texture->Release();
            }
        }
        m_textures.clear();
        return true;
    }

    TextureID TextureManager::CreateTextureFromFile(const std::string& file_name)
    {
        std::string texture_file_path = PathUtils::GetTextureDir() + file_name;

        // 创建纹理资源
        ID3D11Resource* d3d_resource = nullptr;
        ID3D11ShaderResourceView* d3d_shader_resource_view = nullptr;
        std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);
        
        HRESULT hr = DirectX::CreateDDSTextureFromFile(
            g_dolas_engine.m_rhi->m_d3d_device,
            texture_file_path_w.c_str(),
            &d3d_resource,                    // 正确的参数类型
            &d3d_shader_resource_view);

        if (FAILED(hr)) {
            std::cout << "Failed to load texture: " << texture_file_path 
                      << " HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
            return TEXTURE_ID_EMPTY;
        }

        // 获取ID3D11Texture2D接口
        ID3D11Texture2D* d3d_texture_2d = nullptr;
        hr = d3d_resource->QueryInterface(__uuidof(ID3D11Texture2D), 
                                         reinterpret_cast<void**>(&d3d_texture_2d));
        if (FAILED(hr)) {
            std::cout << "Failed to query ID3D11Texture2D interface!" << std::endl;
            d3d_resource->Release();
            if (d3d_shader_resource_view) d3d_shader_resource_view->Release();
            return TEXTURE_ID_EMPTY;
        }

        // 释放原始资源引用（已经通过QueryInterface获得了新的引用）
        d3d_resource->Release();

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = true;
        texture->m_file_id = STRING_ID(texture_file_path);
        texture->m_d3d_texture_2d = d3d_texture_2d;
        texture->m_d3d_shader_resource_view = d3d_shader_resource_view;
        
        m_textures[texture->m_file_id] = texture;
        
        std::cout << "Successfully loaded texture: " << texture_file_path << std::endl;
        return texture->m_file_id;
    }
    
    TextureID TextureManager::CreateTexture2D(
        ID3D11Device* device, 
        const TextureDescriptor& desc,
        const D3D11_SUBRESOURCE_DATA* initData)
    {
        D3D11_USAGE Usage;
        UINT BindFlags;
        UINT CPUAccessFlags;
        UINT MiscFlags;
        
        // 核心类型映射
        switch (desc.usage) {
        case TextureUsage::Immutable:
            Usage = D3D11_USAGE_IMMUTABLE;
            BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case TextureUsage::Dynamic:
            Usage = D3D11_USAGE_DYNAMIC;
            BindFlags = D3D11_BIND_SHADER_RESOURCE;
            CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;
        case TextureUsage::RenderTarget:
            Usage = D3D11_USAGE_DEFAULT;
            BindFlags = D3D11_BIND_RENDER_TARGET;
            if (desc.shaderResource || desc.generateMips) {
                BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            if (desc.generateMips) {
                MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            }
            break;
        case TextureUsage::DepthStencil:
            Usage = D3D11_USAGE_DEFAULT;
            BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if (desc.shaderResource && IsDepthFormatShaderCompatible(ConvertToDXGIFormat(desc.format))) {
                BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            break;
        case TextureUsage::Staging:
            Usage = D3D11_USAGE_STAGING;
            CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            break;
        }
    
        return CreateTexture(
            TextureType::TEXTURE_2D,
            desc.format,
            desc.width,
            desc.height,
            desc.generateMips ? 0 : 1,
            desc.sampleCount,
            0, // sample_quality
            desc.arraySize,
            BindFlags,
            CPUAccessFlags,
            MiscFlags,
            Usage);
    }

    TextureID TextureManager::CreateTexture(
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
    )
    {
        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = false;
        texture->m_file_id = TEXTURE_ID_EMPTY;
        texture->m_texture_type = texture_type;
        texture->m_texture_format = texture_format;
        texture->m_width = width;
        texture->m_height = height;
        texture->m_mip_levels = mip_levels;

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = mip_levels;
        desc.ArraySize = array_size;
        desc.Format = ConvertToDXGIFormat(texture_format);
        desc.SampleDesc.Count = sample_count;
        desc.SampleDesc.Quality = sample_quality;
        desc.Usage = usage;
        desc.BindFlags = bind_flags;
        desc.CPUAccessFlags = cpu_access_flags;
        desc.MiscFlags = misc_flags;

        g_dolas_engine.m_rhi->m_d3d_device->CreateTexture2D(&desc, nullptr, &texture->m_d3d_texture_2d);
        
        m_textures[texture->m_file_id] = texture;
        return texture->m_file_id;
    }
    
    Texture* TextureManager::GetTexture(TextureID texture_id)
    {
        if (m_textures.find(texture_id) == m_textures.end())
        {
            return nullptr;
        }
        return m_textures[texture_id];
    }

    DXGI_FORMAT TextureManager::ConvertToDXGIFormat(TextureFormat format)
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

    Bool TextureManager::IsDepthFormatShaderCompatible(DXGI_FORMAT format)
    {
        switch (format)
        {
            // 支持深度读取的标准格式
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:

        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:

        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:

        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            return true;

            // 不支持深度读取的格式
        // case DXGI_FORMAT_D16_UNORM_S8_UINT:   // 无对应SRV格式
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT: // 纯模板，不支持深度读取
        default:
            return false;
        }
    }
} // namespace Dolas

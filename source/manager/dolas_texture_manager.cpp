
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
        texture->m_file_id = STRING_ID(texture_file_path);
        texture->m_d3d_texture_2d = d3d_texture_2d;
        texture->m_d3d_shader_resource_view = d3d_shader_resource_view;
        
        m_textures[texture->m_file_id] = texture;
        
        std::cout << "Successfully loaded texture: " << texture_file_path << std::endl;
        return texture->m_file_id;
    }
    
    TextureID TextureManager::CreateTexture()
    {
        Texture* texture = DOLAS_NEW(Texture);
        texture->m_file_id = TEXTURE_ID_EMPTY;
        texture->m_texture_type = TextureType::TEXTURE_2D;
        texture->m_texture_format = TextureFormat::R8G8B8A8_UNORM;
        texture->m_width = 1024;
        texture->m_height = 1024;
        texture->m_mip_levels = 1;

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = texture->m_width;
        desc.Height = texture->m_height;
        desc.MipLevels = texture->m_mip_levels;
        desc.ArraySize = 1;

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

} // namespace Dolas

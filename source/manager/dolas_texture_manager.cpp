
#include <iostream>
#include <fstream>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_texture_manager.h"
#include "render/dolas_texture.h"
#include "base/dolas_base.h"
#include "base/dolas_string_util.h"
#include "base/dolas_paths.h"
#include "DirectXTex.h"
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
    // Helper: set D3D11 debug name for RenderDoc and debug layer
    static void SetD3DDebugName(ID3D11DeviceChild* object, const std::string& name)
    {
    #if defined(DEBUG) || defined(_DEBUG)
        if (object && !name.empty())
        {
            object->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
        }
    #else
        (void)object; (void)name;
    #endif
    }
    TextureManager::TextureManager()
    {

    }

    TextureManager::~TextureManager()
    {

    }
    
    bool TextureManager::Initialize()
    {
        // initialize global textures
		//TextureID sky_box_texture_id = CreateTextureFromDDSFile("sky_box.dds");
        // 使用 HDR 文件加载天空盒纹理
        TextureID sky_box_texture_id = CreateTextureFromHDRFile("golden_gate_hills_4k.hdr");
        
        m_global_textures[GlobalTextureType::GLOBAL_TEXTURE_SKY_BOX] = sky_box_texture_id;

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

    Texture* TextureManager::GetTextureByTextureID(TextureID texture_id)
    {
        if (m_textures.find(texture_id) == m_textures.end())
        {
            return nullptr;
        }
        return m_textures[texture_id];
    }

    TextureID TextureManager::CreateTextureFromDDSFile(const std::string& file_name)
    {
        std::string texture_file_path = PathUtils::GetTextureDir() + file_name;

        // 创建纹理资源
        ID3D11Resource* d3d_resource = nullptr;
        ID3D11ShaderResourceView* d3d_shader_resource_view = nullptr;
        std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);
        
        // 使用 DirectXTex 的现代 API
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage image;
        
        // 从 DDS 文件加载
        HRESULT hr = DirectX::LoadFromDDSFile(
            texture_file_path_w.c_str(),
            DirectX::DDS_FLAGS_NONE,
            &metadata,
            image);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to load DDS file: {0} ", texture_file_path);
            return TEXTURE_ID_EMPTY;
        }

        // 创建纹理和 Shader Resource View
        hr = DirectX::CreateTexture(
            g_dolas_engine.m_rhi->GetD3D11Device(),
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &d3d_resource);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to create texture: {0} ", texture_file_path);
            return TEXTURE_ID_EMPTY;
        }

        // 创建 Shader Resource View
        hr = DirectX::CreateShaderResourceView(
            g_dolas_engine.m_rhi->GetD3D11Device(),
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &d3d_shader_resource_view);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to create shader resource view: {0}", texture_file_path);
            if (d3d_resource) {
                d3d_resource->Release();
            }
            return TEXTURE_ID_EMPTY;
        }

        // 获取ID3D11Texture2D接口
        ID3D11Texture2D* d3d_texture_2d = nullptr;
        hr = d3d_resource->QueryInterface(__uuidof(ID3D11Texture2D), 
                                         reinterpret_cast<void**>(&d3d_texture_2d));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to query ID3D11Texture2D interface!");
            d3d_resource->Release();
            if (d3d_shader_resource_view) d3d_shader_resource_view->Release();
            return TEXTURE_ID_EMPTY;
        }

        // 释放原始资源引用（已经通过QueryInterface获得了新的引用）
        d3d_resource->Release();

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = true;
        // 使用运行时文件路径字符串计算唯一哈希，不能用 STRING_ID 宏（那只对编译期字面量安全）
        texture->m_file_id = HashConverter::StringHash(texture_file_path);
        texture->m_d3d_texture_2d = d3d_texture_2d;
        texture->m_d3d_shader_resource_view = d3d_shader_resource_view;

        // Debug names for RenderDoc
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D: ") + texture_file_path);
        SetD3DDebugName(texture->m_d3d_shader_resource_view, std::string("SRV: ") + texture_file_path);
        
        m_textures[texture->m_file_id] = texture;
        
        LOG_INFO("Successfully loaded texture: {0}", texture_file_path);
        return texture->m_file_id;
    }

    TextureID TextureManager::CreateTextureFromHDRFile(const std::string& file_name)
    {
        std::string texture_file_path = PathUtils::GetTextureDir() + file_name;

        // 创建纹理资源
        ID3D11Resource* d3d_resource = nullptr;
        ID3D11ShaderResourceView* d3d_shader_resource_view = nullptr;
        std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);
        
        // 使用 DirectXTex 的 HDR 加载 API
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage image;
        
        // 从 HDR 文件加载
        HRESULT hr = DirectX::LoadFromHDRFile(
            texture_file_path_w.c_str(),
            &metadata,
            image);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to load HDR file: {0}", texture_file_path);
            return TEXTURE_ID_EMPTY;
        }

        // 创建纹理
        hr = DirectX::CreateTexture(
            g_dolas_engine.m_rhi->GetD3D11Device(),
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &d3d_resource);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to create texture from HDR: {0}", texture_file_path);
            return TEXTURE_ID_EMPTY;
        }

        // 创建 Shader Resource View
        hr = DirectX::CreateShaderResourceView(
            g_dolas_engine.m_rhi->GetD3D11Device(),
            image.GetImages(),
            image.GetImageCount(),
            metadata,
            &d3d_shader_resource_view);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to create shader resource view from HDR: {0}", texture_file_path);
            if (d3d_resource) {
                d3d_resource->Release();
            }
            return TEXTURE_ID_EMPTY;
        }

        // 获取ID3D11Texture2D接口
        ID3D11Texture2D* d3d_texture_2d = nullptr;
        hr = d3d_resource->QueryInterface(__uuidof(ID3D11Texture2D), 
                                         reinterpret_cast<void**>(&d3d_texture_2d));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to query ID3D11Texture2D interface from HDR!");
            d3d_resource->Release();
            if (d3d_shader_resource_view) d3d_shader_resource_view->Release();
            return TEXTURE_ID_EMPTY;
        }

        // 释放原始资源引用（已经通过QueryInterface获得了新的引用）
        d3d_resource->Release();

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = true;
        // 使用运行时文件路径字符串计算唯一哈希，不能用 STRING_ID 宏（那只对编译期字面量安全）
        texture->m_file_id = HashConverter::StringHash(texture_file_path);
        texture->m_d3d_texture_2d = d3d_texture_2d;
        texture->m_d3d_shader_resource_view = d3d_shader_resource_view;

        // Debug names for RenderDoc
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D_HDR: ") + texture_file_path);
        SetD3DDebugName(texture->m_d3d_shader_resource_view, std::string("SRV_HDR: ") + texture_file_path);
        
        m_textures[texture->m_file_id] = texture;
        
        LOG_INFO("Successfully loaded HDR texture: {0}", texture_file_path);
        return texture->m_file_id;
    }

    Texture* TextureManager::GetGlobalTexture(GlobalTextureType global_texture_type)
    {
		Texture* result_texture = nullptr;

        auto type_iter = m_global_textures.find(global_texture_type);
        if (type_iter != m_global_textures.end())
        {
            TextureID texture_id = type_iter->second;

            auto texture_iter = m_textures.find(texture_id);
            if (texture_iter != m_textures.end())
            {
                result_texture = texture_iter->second;
            }
        }
		return result_texture;
    }

    Bool TextureManager::DolasCreateTexture2D(const DolasTexture2DDesc& dolas_texture2d_desc)
    {
        D3D11_TEXTURE2D_DESC d3d_texture2d_desc = (D3D11_TEXTURE2D_DESC)0;
		d3d_texture2d_desc.Width = dolas_texture2d_desc.width;
		d3d_texture2d_desc.Height = dolas_texture2d_desc.height;
		d3d_texture2d_desc.MipLevels = dolas_texture2d_desc.generateMips ? 0 : 1; // 0表示自动生成所有mip级别
		d3d_texture2d_desc.ArraySize = dolas_texture2d_desc.arraySize;
		d3d_texture2d_desc.Format = ConvertToDXGIFormat(dolas_texture2d_desc.format);
		d3d_texture2d_desc.SampleDesc.Count = dolas_texture2d_desc.sampleCount;
		d3d_texture2d_desc.SampleDesc.Quality = 0; // 默认质量级别
        d3d_texture2d_desc.Usage = D3D11_USAGE_DEFAULT; // 默认使用方式
		d3d_texture2d_desc.BindFlags = 0;
		d3d_texture2d_desc.CPUAccessFlags = 0;
		d3d_texture2d_desc.MiscFlags = 0;
        // 核心类型映射
        switch (dolas_texture2d_desc.usage) {
        case DolasTextureUsage::Immutable:
            d3d_texture2d_desc.Usage = D3D11_USAGE_IMMUTABLE;
            d3d_texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;
        case DolasTextureUsage::Dynamic:
            d3d_texture2d_desc.Usage = D3D11_USAGE_DYNAMIC;
            d3d_texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            d3d_texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;
        case DolasTextureUsage::RenderTarget:
            d3d_texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
            d3d_texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
            if (dolas_texture2d_desc.shaderResource || dolas_texture2d_desc.generateMips) {
                d3d_texture2d_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            if (dolas_texture2d_desc.generateMips) {
                d3d_texture2d_desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            }
            break;
        case DolasTextureUsage::DepthStencil:
            d3d_texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
            d3d_texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if (dolas_texture2d_desc.shaderResource && IsDepthFormatShaderCompatible(ConvertToDXGIFormat(dolas_texture2d_desc.format))) {
                d3d_texture2d_desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            break;
        case DolasTextureUsage::Staging:
            d3d_texture2d_desc.Usage = D3D11_USAGE_STAGING;
            d3d_texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            break;
        }

        return D3DCreateTexture2D(dolas_texture2d_desc.texture_handle, &d3d_texture2d_desc);
    }
    DXGI_FORMAT TextureManager::ConvertToDXGIFormat(DolasTextureFormat texture_format)
    {
        switch (texture_format)
        {
            case DolasTextureFormat::R8G8B8A8_UNORM:         return DXGI_FORMAT_R8G8B8A8_UNORM;
            case DolasTextureFormat::R8G8B8A8_SRGB:          return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case DolasTextureFormat::BC1_UNORM:              return DXGI_FORMAT_BC1_UNORM;
            case DolasTextureFormat::BC1_SRGB:               return DXGI_FORMAT_BC1_UNORM_SRGB;
            case DolasTextureFormat::BC3_UNORM:              return DXGI_FORMAT_BC3_UNORM;
            case DolasTextureFormat::BC3_SRGB:               return DXGI_FORMAT_BC3_UNORM_SRGB;
            case DolasTextureFormat::BC7_UNORM:              return DXGI_FORMAT_BC7_UNORM;
            case DolasTextureFormat::BC7_SRGB:               return DXGI_FORMAT_BC7_UNORM_SRGB;
            case DolasTextureFormat::R32G32B32A32_FLOAT:     return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case DolasTextureFormat::R16G16B16A16_FLOAT:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case DolasTextureFormat::D24_UNORM_S8_UINT:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DolasTextureFormat::D32_FLOAT:              return DXGI_FORMAT_D32_FLOAT;
            case DolasTextureFormat::R32_TYPELESS:           return DXGI_FORMAT_R32_TYPELESS;
            case DolasTextureFormat::R24G8_TYPELESS:         return DXGI_FORMAT_R24G8_TYPELESS;
            case DolasTextureFormat::R16_TYPELESS:           return DXGI_FORMAT_R16_TYPELESS;
            case DolasTextureFormat::R32G8X24_TYPELESS:      return DXGI_FORMAT_R32G8X24_TYPELESS;
            default:                                    return DXGI_FORMAT_R8G8B8A8_UNORM;
        }
    }

    DolasTextureFormat TextureManager::ConvertToTextureFormat(DXGI_FORMAT dxgi_format)
    {
        switch (dxgi_format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:            return DolasTextureFormat::R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:       return DolasTextureFormat::R8G8B8A8_SRGB;
        case DXGI_FORMAT_BC1_UNORM:                 return DolasTextureFormat::BC1_UNORM;
        case DXGI_FORMAT_BC1_UNORM_SRGB:            return DolasTextureFormat::BC1_SRGB;
        case DXGI_FORMAT_BC3_UNORM:                 return DolasTextureFormat::BC3_UNORM;
        case DXGI_FORMAT_BC3_UNORM_SRGB:            return DolasTextureFormat::BC3_SRGB;
        case DXGI_FORMAT_BC7_UNORM:                 return DolasTextureFormat::BC7_UNORM;
        case DXGI_FORMAT_BC7_UNORM_SRGB:            return DolasTextureFormat::BC7_SRGB;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:        return DolasTextureFormat::R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:        return DolasTextureFormat::R16G16B16A16_FLOAT;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:         return DolasTextureFormat::D24_UNORM_S8_UINT;
        case DXGI_FORMAT_D32_FLOAT:                 return DolasTextureFormat::D32_FLOAT;
        case DXGI_FORMAT_R32_TYPELESS:              return DolasTextureFormat::R32_TYPELESS;
        case DXGI_FORMAT_R24G8_TYPELESS:            return DolasTextureFormat::R24G8_TYPELESS;
        case DXGI_FORMAT_R16_TYPELESS:              return DolasTextureFormat::R16_TYPELESS;
        case DXGI_FORMAT_R32G8X24_TYPELESS:         return DolasTextureFormat::R32G8X24_TYPELESS;
        default:                                    return DolasTextureFormat::R8G8B8A8_UNORM;
        }
    }

    Bool TextureManager::D3DCreateTexture2D(
        TextureID texture_handle,
        const D3D11_TEXTURE2D_DESC* pDesc)
    {
        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = false;
        texture->m_file_id = TEXTURE_ID_EMPTY;
        texture->m_texture_type = DolasTextureType::TEXTURE_2D;
        texture->m_texture_format = ConvertToTextureFormat(pDesc->Format);
        texture->m_width = pDesc->Width;
        texture->m_height = pDesc->Height;
        texture->m_mip_levels = pDesc->MipLevels;

        g_dolas_engine.m_rhi->GetD3D11Device()->CreateTexture2D(pDesc, nullptr, &texture->m_d3d_texture_2d);

        // Debug name using string id registry if present
        std::string tex_name = ID_TO_STRING(texture_handle);
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D: ") + tex_name);

        m_textures[texture_handle] = texture;

        return true;
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

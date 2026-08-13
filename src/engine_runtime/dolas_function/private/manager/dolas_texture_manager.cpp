
#include <iostream>
#include <fstream>

#include <d3d11.h>

#include "dolas_engine.h"
#include "dolas_render_hardware_interface.h"
#include "render/dolas_rhi.h"
#include "manager/dolas_texture_manager.h"
#include "render/dolas_texture.h"
#include "dolas_base.h"
#include "dolas_string_util.h"
#include "dolas_paths.h"
#include <algorithm>
#include <cstring>
#include <vector>
#include "DirectXTex.h"
#include "dolas_log_system_manager.h"
#include "render/dolas_dx_trace.h"
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

    static void SetD3D12DebugName(ID3D12Object* object, const std::wstring& name)
    {
    #if defined(DEBUG) || defined(_DEBUG)
        if (object && !name.empty())
        {
            object->SetName(name.c_str());
        }
    #else
        (void)object; (void)name;
    #endif
    }

    template<typename T>
    static void SafeRelease(T*& ptr)
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    static D3D12_RESOURCE_DIMENSION ConvertToD3D12ResourceDimension(DirectX::TEX_DIMENSION dimension)
    {
        switch (dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case DirectX::TEX_DIMENSION_TEXTURE2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case DirectX::TEX_DIMENSION_TEXTURE3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:                               return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        }
    }

    static DolasTextureType ConvertToDolasTextureType(const DirectX::TexMetadata& metadata)
    {
        if (metadata.IsCubemap())
        {
            return DolasTextureType::TEXTURE_CUBE;
        }

        if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
        {
            return DolasTextureType::TEXTURE_3D;
        }

        if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D && metadata.arraySize > 1)
        {
            return DolasTextureType::TEXTURE_ARRAY_2D;
        }

        return DolasTextureType::TEXTURE_2D;
    }

    static DXGI_FORMAT ConvertDepthResourceToDsvFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R24G8_TYPELESS:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_R32_TYPELESS:      return DXGI_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_R16_TYPELESS:      return DXGI_FORMAT_D16_UNORM;
        case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:                            return format;
        }
    }

    static DXGI_FORMAT ConvertDepthResourceToSrvFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_D16_UNORM:
            return DXGI_FORMAT_R16_UNORM;
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default:
            return format;
        }
    }

    static D3D12_SHADER_RESOURCE_VIEW_DESC CreateD3D12SrvDescFromMetadata(const DirectX::TexMetadata& metadata)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = metadata.format;
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        const UINT mip_levels = static_cast<UINT>(std::max<size_t>(1, metadata.mipLevels));
        const UINT array_size = static_cast<UINT>(std::max<size_t>(1, metadata.arraySize));

        switch (metadata.dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE1D:
            if (array_size > 1)
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.MipLevels = mip_levels;
                srv_desc.Texture1DArray.ArraySize = array_size;
            }
            else
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MipLevels = mip_levels;
            }
            break;
        case DirectX::TEX_DIMENSION_TEXTURE3D:
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srv_desc.Texture3D.MipLevels = mip_levels;
            break;
        case DirectX::TEX_DIMENSION_TEXTURE2D:
        default:
            if (metadata.IsCubemap())
            {
                const UINT cube_count = std::max<UINT>(1, array_size / 6);
                if (cube_count > 1)
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    srv_desc.TextureCubeArray.MipLevels = mip_levels;
                    srv_desc.TextureCubeArray.NumCubes = cube_count;
                }
                else
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srv_desc.TextureCube.MipLevels = mip_levels;
                }
            }
            else if (array_size > 1)
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srv_desc.Texture2DArray.MipLevels = mip_levels;
                srv_desc.Texture2DArray.ArraySize = array_size;
            }
            else
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MipLevels = mip_levels;
            }
            break;
        }

        return srv_desc;
    }

    static D3D12_SHADER_RESOURCE_VIEW_DESC CreateD3D12Texture2DSrvDesc(
        DXGI_FORMAT format,
        UINT mip_levels,
        UINT array_size,
        UINT sample_count)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = ConvertDepthResourceToSrvFormat(format);
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (sample_count > 1)
        {
            if (array_size > 1)
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srv_desc.Texture2DMSArray.ArraySize = array_size;
            }
            else
            {
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
        }
        else if (array_size > 1)
        {
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srv_desc.Texture2DArray.MipLevels = std::max<UINT>(1, mip_levels);
            srv_desc.Texture2DArray.ArraySize = array_size;
        }
        else
        {
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MipLevels = std::max<UINT>(1, mip_levels);
        }

        return srv_desc;
    }

    static bool CreateD3D12TextureFromScratchImage(
        const DirectX::ScratchImage& image,
        const DirectX::TexMetadata& metadata,
        const std::wstring& debug_name,
        Texture* texture)
    {
        RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
        ID3D12Device* device = rhi ? rhi->GetDevice() : nullptr;
        if (!rhi || !device || !texture)
        {
            LOG_ERROR("CreateD3D12TextureFromScratchImage: D3D12 device or texture is null");
            return false;
        }

        const UINT subresource_count = static_cast<UINT>(image.GetImageCount());
        const DirectX::Image* images = image.GetImages();
        if (subresource_count == 0 || !images)
        {
            LOG_ERROR("CreateD3D12TextureFromScratchImage: ScratchImage has no image data");
            return false;
        }

        D3D12_RESOURCE_DESC texture_desc = {};
        texture_desc.Dimension = ConvertToD3D12ResourceDimension(metadata.dimension);
        texture_desc.Width = static_cast<UINT64>(metadata.width);
        texture_desc.Height = static_cast<UINT>(metadata.height);
        texture_desc.DepthOrArraySize = static_cast<UINT16>(
            metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D ? metadata.depth : metadata.arraySize);
        texture_desc.MipLevels = static_cast<UINT16>(std::max<size_t>(1, metadata.mipLevels));
        texture_desc.Format = metadata.format;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.SampleDesc.Quality = 0;
        texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES default_heap_properties = {};
        default_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        default_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        default_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        default_heap_properties.CreationNodeMask = 1;
        default_heap_properties.VisibleNodeMask = 1;

        ID3D12Resource* d3d12_resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &default_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &texture_desc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&d3d12_resource));
        if (FAILED(hr))
        {
            LOG_ERROR("CreateD3D12TextureFromScratchImage: failed to create texture, HRESULT: 0x{0:X}", hr);
            return false;
        }

        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(subresource_count);
        std::vector<UINT> num_rows(subresource_count);
        std::vector<UINT64> row_sizes_in_bytes(subresource_count);
        UINT64 upload_buffer_size = 0;
        device->GetCopyableFootprints(
            &texture_desc,
            0,
            subresource_count,
            0,
            layouts.data(),
            num_rows.data(),
            row_sizes_in_bytes.data(),
            &upload_buffer_size);

        D3D12_RESOURCE_DESC upload_desc = {};
        upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        upload_desc.Width = upload_buffer_size;
        upload_desc.Height = 1;
        upload_desc.DepthOrArraySize = 1;
        upload_desc.MipLevels = 1;
        upload_desc.Format = DXGI_FORMAT_UNKNOWN;
        upload_desc.SampleDesc.Count = 1;
        upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES upload_heap_properties = {};
        upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        upload_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        upload_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        upload_heap_properties.CreationNodeMask = 1;
        upload_heap_properties.VisibleNodeMask = 1;

        ID3D12Resource* upload_resource = nullptr;
        hr = device->CreateCommittedResource(
            &upload_heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &upload_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&upload_resource));
        if (FAILED(hr))
        {
            LOG_ERROR("CreateD3D12TextureFromScratchImage: failed to create upload resource, HRESULT: 0x{0:X}", hr);
            SafeRelease(d3d12_resource);
            return false;
        }

        uint8_t* mapped_data = nullptr;
        D3D12_RANGE read_range = { 0, 0 };
        hr = upload_resource->Map(0, &read_range, reinterpret_cast<void**>(&mapped_data));
        if (FAILED(hr))
        {
            LOG_ERROR("CreateD3D12TextureFromScratchImage: failed to map upload resource, HRESULT: 0x{0:X}", hr);
            SafeRelease(upload_resource);
            SafeRelease(d3d12_resource);
            return false;
        }

        for (UINT subresource_index = 0; subresource_index < subresource_count; ++subresource_index)
        {
            const DirectX::Image& source_image = images[subresource_index];
            const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[subresource_index];
            uint8_t* destination_subresource = mapped_data + layout.Offset;
            const UINT64 row_size = std::min<UINT64>(row_sizes_in_bytes[subresource_index], source_image.rowPitch);
            const UINT depth = std::max<UINT>(1, layout.Footprint.Depth);

            for (UINT z = 0; z < depth; ++z)
            {
                uint8_t* destination_slice = destination_subresource +
                    static_cast<size_t>(z) * layout.Footprint.RowPitch * num_rows[subresource_index];
                const uint8_t* source_slice = source_image.pixels + static_cast<size_t>(z) * source_image.slicePitch;

                for (UINT row = 0; row < num_rows[subresource_index]; ++row)
                {
                    memcpy(
                        destination_slice + static_cast<size_t>(row) * layout.Footprint.RowPitch,
                        source_slice + static_cast<size_t>(row) * source_image.rowPitch,
                        static_cast<size_t>(row_size));
                }
            }
        }

        D3D12_RANGE written_range = { 0, static_cast<SIZE_T>(upload_buffer_size) };
        upload_resource->Unmap(0, &written_range);

        const bool submitted = rhi->ExecuteImmediate([&](ID3D12GraphicsCommandList* command_list) -> bool
        {
            for (UINT subresource_index = 0; subresource_index < subresource_count; ++subresource_index)
            {
                D3D12_TEXTURE_COPY_LOCATION destination = {};
                destination.pResource = d3d12_resource;
                destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                destination.SubresourceIndex = subresource_index;

                D3D12_TEXTURE_COPY_LOCATION source = {};
                source.pResource = upload_resource;
                source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                source.PlacedFootprint = layouts[subresource_index];

                command_list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
            }

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = d3d12_resource;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            command_list->ResourceBarrier(1, &barrier);
            return true;
        });

        SafeRelease(upload_resource);
        if (!submitted)
        {
            SafeRelease(d3d12_resource);
            return false;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle = {};
        if (!rhi->AllocateSrvDescriptor(&srv_cpu_handle, &srv_gpu_handle))
        {
            SafeRelease(d3d12_resource);
            return false;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = CreateD3D12SrvDescFromMetadata(metadata);
        device->CreateShaderResourceView(d3d12_resource, &srv_desc, srv_cpu_handle);

        texture->SetD3D12Resource(d3d12_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        texture->SetD3D12SrvHandles(srv_cpu_handle, srv_gpu_handle);
        SetD3D12DebugName(texture->GetD3D12Resource(), debug_name);
        return true;
    }

    static bool CreateD3D12TextureFromD3D11Desc(Texture* texture, const D3D11_TEXTURE2D_DESC* d3d11_desc)
    {
        RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
        ID3D12Device* device = rhi ? rhi->GetDevice() : nullptr;
        if (!rhi || !device || !texture || !d3d11_desc)
        {
            LOG_ERROR("CreateD3D12TextureFromD3D11Desc: invalid input");
            return false;
        }

        const bool is_render_target = (d3d11_desc->BindFlags & D3D11_BIND_RENDER_TARGET) != 0;
        const bool is_depth_stencil = (d3d11_desc->BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0;
        const bool is_shader_resource = (d3d11_desc->BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0;

        D3D12_RESOURCE_FLAGS resource_flags = D3D12_RESOURCE_FLAG_NONE;
        if (is_render_target)
        {
            resource_flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (is_depth_stencil)
        {
            resource_flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        D3D12_RESOURCE_DESC resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_desc.Width = d3d11_desc->Width;
        resource_desc.Height = d3d11_desc->Height;
        resource_desc.DepthOrArraySize = static_cast<UINT16>(std::max<UINT>(1, d3d11_desc->ArraySize));
        resource_desc.MipLevels = static_cast<UINT16>(std::max<UINT>(1, d3d11_desc->MipLevels));
        resource_desc.Format = d3d11_desc->Format;
        resource_desc.SampleDesc = d3d11_desc->SampleDesc;
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_desc.Flags = resource_flags;

        D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COMMON;
        D3D12_CLEAR_VALUE clear_value = {};
        D3D12_CLEAR_VALUE* clear_value_ptr = nullptr;

        if (is_depth_stencil)
        {
            initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            clear_value.Format = ConvertDepthResourceToDsvFormat(d3d11_desc->Format);
            clear_value.DepthStencil.Depth = 1.0f;
            clear_value.DepthStencil.Stencil = 0;
            clear_value_ptr = &clear_value;
        }
        else if (is_render_target)
        {
            initial_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
            clear_value.Format = d3d11_desc->Format;
            clear_value.Color[0] = 0.0f;
            clear_value.Color[1] = 0.0f;
            clear_value.Color[2] = 0.0f;
            clear_value.Color[3] = 1.0f;
            clear_value_ptr = &clear_value;
        }
        else if (is_shader_resource)
        {
            initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        }

        D3D12_HEAP_PROPERTIES heap_properties = {};
        heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 1;
        heap_properties.VisibleNodeMask = 1;

        ID3D12Resource* d3d12_resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &resource_desc,
            initial_state,
            clear_value_ptr,
            IID_PPV_ARGS(&d3d12_resource));
        if (FAILED(hr))
        {
            LOG_ERROR("CreateD3D12TextureFromD3D11Desc: failed to create texture, HRESULT: 0x{0:X}", hr);
            return false;
        }

        if (is_render_target)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = {};
            if (!rhi->AllocateRtvDescriptor(&rtv_handle))
            {
                SafeRelease(d3d12_resource);
                return false;
            }

            D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
            rtv_desc.Format = d3d11_desc->Format;
            rtv_desc.ViewDimension = d3d11_desc->SampleDesc.Count > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;
            device->CreateRenderTargetView(d3d12_resource, &rtv_desc, rtv_handle);
            texture->SetD3D12RtvHandle(rtv_handle);
        }

        if (is_depth_stencil)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = {};
            if (!rhi->AllocateDsvDescriptor(&dsv_handle))
            {
                SafeRelease(d3d12_resource);
                return false;
            }

            D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
            dsv_desc.Format = ConvertDepthResourceToDsvFormat(d3d11_desc->Format);
            dsv_desc.ViewDimension = d3d11_desc->SampleDesc.Count > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
            device->CreateDepthStencilView(d3d12_resource, &dsv_desc, dsv_handle);
            texture->SetD3D12DsvHandle(dsv_handle);
        }

        if (is_shader_resource)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle = {};
            D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle = {};
            if (!rhi->AllocateSrvDescriptor(&srv_cpu_handle, &srv_gpu_handle))
            {
                SafeRelease(d3d12_resource);
                return false;
            }

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = CreateD3D12Texture2DSrvDesc(
                d3d11_desc->Format,
                d3d11_desc->MipLevels,
                d3d11_desc->ArraySize,
                d3d11_desc->SampleDesc.Count);
            device->CreateShaderResourceView(d3d12_resource, &srv_desc, srv_cpu_handle);
            texture->SetD3D12SrvHandles(srv_cpu_handle, srv_gpu_handle);
        }

        texture->SetD3D12Resource(d3d12_resource, initial_state);
        return true;
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
        // 使用 HDR 文件加载天空盒纹理
        const auto sky_box_asset_path = AssetPath::Parse("_engine/texture/golden_gate_hills_4k.hdr");
        if (!sky_box_asset_path)
        {
            LOG_ERROR("TextureManager::Initialize: invalid skybox asset path");
            return false;
        }

        TextureID sky_box_texture_id = CreateTextureFromHDRFile(*sky_box_asset_path);
        if (sky_box_texture_id == TEXTURE_ID_EMPTY)
        {
            LOG_ERROR("TextureManager::Initialize: failed to load required skybox texture");
            return false;
        }
        
        m_global_textures[GlobalTextureType::GLOBAL_TEXTURE_SKY_BOX] = sky_box_texture_id;

        return true;
    }

    bool TextureManager::Clear()
    {
        for (auto texture_iter = m_textures.begin(); texture_iter != m_textures.end(); ++texture_iter)
        {
            Texture* texture = texture_iter->second;
            if (texture)
            {
                texture->Release();
                DOLAS_DELETE(texture);
            }
        }
        m_textures.clear();
        m_global_textures.clear();
        return true;
    }

    Texture* TextureManager::GetTextureByTextureID(TextureID texture_id)
    {
        auto it = m_textures.find(texture_id);
        return (it != m_textures.end()) ? it->second : nullptr;
    }

    Bool TextureManager::DestroyTextureByID(TextureID texture_id)
    {
        auto texture_iter = m_textures.find(texture_id);
        if (texture_iter == m_textures.end())
        {
            return false;
        }

        Texture* texture = texture_iter->second;
        if (texture)
        {
            texture->Release();
            DOLAS_DELETE(texture);
        }
        m_textures.erase(texture_iter);

        return true;
    }

    TextureID TextureManager::CreateTextureFromDDSFile(const AssetPath& asset_path)
    {
        const auto resolved_path = PathUtils::ResolveAssetPath(asset_path);
        if (!resolved_path)
        {
            LOG_ERROR("TextureManager::CreateTextureFromDDSFile: failed to resolve {0}", asset_path.GetCanonicalPath());
            return TEXTURE_ID_EMPTY;
        }

        const std::string texture_file_path = resolved_path->string();
        const std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);
        
        // 使用 DirectXTex 的现代 API
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage image;
        
        // 从 DDS 文件加载
        HRESULT load_hr = DirectX::LoadFromDDSFile(
            texture_file_path_w.c_str(),
            DirectX::DDS_FLAGS_NONE,
            &metadata,
            image);
        if (FAILED(load_hr))
        {
            LOG_ERROR("TextureManager::CreateTextureFromDDSFile: failed to load {0}, HRESULT: 0x{1:X}", texture_file_path, load_hr);
            return TEXTURE_ID_EMPTY;
        }

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = true;
        // 使用规范逻辑资产路径计算稳定 ID，不依赖本机 Content 根目录。
        texture->m_file_id = HashConverter::StringHash(asset_path.GetCanonicalPath());
        texture->m_texture_type = ConvertToDolasTextureType(metadata);
        texture->m_texture_format = ConvertToTextureFormat(metadata.format);
        texture->m_width = static_cast<uint32_t>(metadata.width);
        texture->m_height = static_cast<uint32_t>(metadata.height);
        texture->m_mip_levels = static_cast<uint32_t>(metadata.mipLevels);
        if (!CreateD3D12TextureFromScratchImage(image, metadata, texture_file_path_w, texture))
        {
            LOG_ERROR("Failed to create D3D12 DDS texture: {0}", texture_file_path);
            texture->Release();
            DOLAS_DELETE(texture);
            return TEXTURE_ID_EMPTY;
        }

        ID3D11Device* d3d11_device = g_dolas_engine.m_rhi ? g_dolas_engine.m_rhi->GetD3D11Device() : nullptr;
        if (d3d11_device)
        {
            ID3D11Resource* d3d_resource = nullptr;
            HRESULT hr = DirectX::CreateTexture(
                d3d11_device,
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                &d3d_resource);
            if (SUCCEEDED(hr) && d3d_resource)
            {
                hr = d3d_resource->QueryInterface(
                    __uuidof(ID3D11Texture2D),
                    reinterpret_cast<void**>(&texture->m_d3d_texture_2d));
                if (FAILED(hr))
                {
                    LOG_WARN("TextureManager::CreateTextureFromDDSFile: failed to create optional D3D11 texture mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
                }
                d3d_resource->Release();
            }
            else
            {
                LOG_WARN("TextureManager::CreateTextureFromDDSFile: failed to create optional D3D11 resource mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
            }

            hr = DirectX::CreateShaderResourceView(
                d3d11_device,
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                &texture->m_d3d_shader_resource_view);
            if (FAILED(hr))
            {
                LOG_WARN("TextureManager::CreateTextureFromDDSFile: failed to create optional D3D11 SRV mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
            }
        }

        // Debug names for RenderDoc
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D: ") + texture_file_path);
        SetD3DDebugName(texture->m_d3d_shader_resource_view, std::string("SRV: ") + texture_file_path);
        
        DestroyTextureByID(texture->m_file_id);
        m_textures[texture->m_file_id] = texture;
        
        LOG_INFO("Successfully loaded texture: {0}", texture_file_path);
        return texture->m_file_id;
    }

    TextureID TextureManager::CreateTextureFromHDRFile(const AssetPath& asset_path)
    {
        const auto resolved_path = PathUtils::ResolveAssetPath(asset_path);
        if (!resolved_path)
        {
            LOG_ERROR("TextureManager::CreateTextureFromHDRFile: failed to resolve {0}", asset_path.GetCanonicalPath());
            return TEXTURE_ID_EMPTY;
        }

        const std::string texture_file_path = resolved_path->string();
        const std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);
        
        // 使用 DirectXTex 的 HDR 加载 API
        DirectX::TexMetadata metadata;
        DirectX::ScratchImage image;
        
        // 从 HDR 文件加载
        HRESULT load_hr = DirectX::LoadFromHDRFile(
            texture_file_path_w.c_str(),
            &metadata,
            image);
        if (FAILED(load_hr))
        {
            LOG_ERROR("TextureManager::CreateTextureFromHDRFile: failed to load {0}, HRESULT: 0x{1:X}", texture_file_path, load_hr);
            return TEXTURE_ID_EMPTY;
        }

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = true;
        // 使用规范逻辑资产路径计算稳定 ID，不依赖本机 Content 根目录。
        texture->m_file_id = HashConverter::StringHash(asset_path.GetCanonicalPath());
        texture->m_texture_type = ConvertToDolasTextureType(metadata);
        texture->m_texture_format = ConvertToTextureFormat(metadata.format);
        texture->m_width = static_cast<uint32_t>(metadata.width);
        texture->m_height = static_cast<uint32_t>(metadata.height);
        texture->m_mip_levels = static_cast<uint32_t>(metadata.mipLevels);
        if (!CreateD3D12TextureFromScratchImage(image, metadata, texture_file_path_w, texture))
        {
            LOG_ERROR("Failed to create D3D12 HDR texture: {0}", texture_file_path);
            texture->Release();
            DOLAS_DELETE(texture);
            return TEXTURE_ID_EMPTY;
        }

        ID3D11Device* d3d11_device = g_dolas_engine.m_rhi ? g_dolas_engine.m_rhi->GetD3D11Device() : nullptr;
        if (d3d11_device)
        {
            ID3D11Resource* d3d_resource = nullptr;
            HRESULT hr = DirectX::CreateTexture(
                d3d11_device,
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                &d3d_resource);
            if (SUCCEEDED(hr) && d3d_resource)
            {
                hr = d3d_resource->QueryInterface(
                    __uuidof(ID3D11Texture2D),
                    reinterpret_cast<void**>(&texture->m_d3d_texture_2d));
                if (FAILED(hr))
                {
                    LOG_WARN("TextureManager::CreateTextureFromHDRFile: failed to create optional D3D11 texture mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
                }
                d3d_resource->Release();
            }
            else
            {
                LOG_WARN("TextureManager::CreateTextureFromHDRFile: failed to create optional D3D11 resource mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
            }

            hr = DirectX::CreateShaderResourceView(
                d3d11_device,
                image.GetImages(),
                image.GetImageCount(),
                metadata,
                &texture->m_d3d_shader_resource_view);
            if (FAILED(hr))
            {
                LOG_WARN("TextureManager::CreateTextureFromHDRFile: failed to create optional D3D11 SRV mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
            }
        }

        // Debug names for RenderDoc
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D_HDR: ") + texture_file_path);
        SetD3DDebugName(texture->m_d3d_shader_resource_view, std::string("SRV_HDR: ") + texture_file_path);
        
        DestroyTextureByID(texture->m_file_id);
        m_textures[texture->m_file_id] = texture;
        
        LOG_INFO("Successfully loaded HDR texture: {0}", texture_file_path);
        return texture->m_file_id;
    }

	TextureID TextureManager::CreateTextureFromPNGFile(const AssetPath& asset_path)
	{
		const auto resolved_path = PathUtils::ResolveAssetPath(asset_path);
		if (!resolved_path)
		{
			LOG_ERROR("TextureManager::CreateTextureFromPNGFile: failed to resolve {0}", asset_path.GetCanonicalPath());
			return TEXTURE_ID_EMPTY;
		}

		const std::string texture_file_path = resolved_path->string();
		const std::wstring texture_file_path_w = StringUtil::StringToWString(texture_file_path);

		// 使用 DirectXTex 的 WIC 加载 API (支持 PNG, JPG, BMP 等)
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;

		// 从 WIC 文件加载
		HRESULT load_hr = DirectX::LoadFromWICFile(
			texture_file_path_w.c_str(),
			DirectX::WIC_FLAGS_NONE,
			&metadata,
			image);
		if (FAILED(load_hr))
		{
			LOG_ERROR("TextureManager::CreateTextureFromPNGFile: failed to load {0}, HRESULT: 0x{1:X}", texture_file_path, load_hr);
			return TEXTURE_ID_EMPTY;
		}

		Texture* texture = DOLAS_NEW(Texture);
		texture->m_is_from_file = true;
		texture->m_file_id = HashConverter::StringHash(asset_path.GetCanonicalPath());
		texture->m_texture_type = ConvertToDolasTextureType(metadata);
		texture->m_texture_format = ConvertToTextureFormat(metadata.format);
		texture->m_width = static_cast<uint32_t>(metadata.width);
		texture->m_height = static_cast<uint32_t>(metadata.height);
		texture->m_mip_levels = static_cast<uint32_t>(metadata.mipLevels);
		if (!CreateD3D12TextureFromScratchImage(image, metadata, texture_file_path_w, texture))
		{
			LOG_ERROR("Failed to create D3D12 PNG texture: {0}", texture_file_path);
			texture->Release();
			DOLAS_DELETE(texture);
			return TEXTURE_ID_EMPTY;
		}

		ID3D11Device* d3d11_device = g_dolas_engine.m_rhi ? g_dolas_engine.m_rhi->GetD3D11Device() : nullptr;
		if (d3d11_device)
		{
			ID3D11Resource* d3d_resource = nullptr;
			HRESULT hr = DirectX::CreateTexture(
				d3d11_device,
				image.GetImages(),
				image.GetImageCount(),
				metadata,
				&d3d_resource);
			if (SUCCEEDED(hr) && d3d_resource)
			{
				hr = d3d_resource->QueryInterface(
					__uuidof(ID3D11Texture2D),
					reinterpret_cast<void**>(&texture->m_d3d_texture_2d));
				if (FAILED(hr))
				{
					LOG_WARN("TextureManager::CreateTextureFromPNGFile: failed to create optional D3D11 texture mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
				}
				d3d_resource->Release();
			}
			else
			{
				LOG_WARN("TextureManager::CreateTextureFromPNGFile: failed to create optional D3D11 resource mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
			}

			hr = DirectX::CreateShaderResourceView(
				d3d11_device,
				image.GetImages(),
				image.GetImageCount(),
				metadata,
				&texture->m_d3d_shader_resource_view);
			if (FAILED(hr))
			{
				LOG_WARN("TextureManager::CreateTextureFromPNGFile: failed to create optional D3D11 SRV mirror for {0}, HRESULT: 0x{1:X}", texture_file_path, hr);
			}
		}

		// Debug names for RenderDoc
		SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D_PNG: ") + texture_file_path);
		SetD3DDebugName(texture->m_d3d_shader_resource_view, std::string("SRV_PNG: ") + texture_file_path);

		DestroyTextureByID(texture->m_file_id);
		m_textures[texture->m_file_id] = texture;

		LOG_INFO("Successfully loaded PNG texture: {0}", texture_file_path);
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
        if (!pDesc)
        {
            LOG_ERROR("TextureManager::D3DCreateTexture2D: texture description is null");
            return false;
        }

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_is_from_file = false;
        texture->m_file_id = TEXTURE_ID_EMPTY;
        texture->m_texture_type = DolasTextureType::TEXTURE_2D;
        texture->m_texture_format = ConvertToTextureFormat(pDesc->Format);
        texture->m_width = pDesc->Width;
        texture->m_height = pDesc->Height;
        texture->m_mip_levels = pDesc->MipLevels;

        if (!CreateD3D12TextureFromD3D11Desc(texture, pDesc))
        {
            LOG_ERROR("TextureManager::D3DCreateTexture2D: Failed to create D3D12 texture");
            texture->Release();
            DOLAS_DELETE(texture);
            return false;
        }

        ID3D11Device* d3d11_device = g_dolas_engine.m_rhi ? g_dolas_engine.m_rhi->GetD3D11Device() : nullptr;
        if (d3d11_device)
        {
            HRESULT hr = d3d11_device->CreateTexture2D(pDesc, nullptr, &texture->m_d3d_texture_2d);
            if (FAILED(hr))
            {
                LOG_WARN("TextureManager::D3DCreateTexture2D: failed to create optional D3D11 texture mirror, HRESULT: 0x{0:X}", hr);
            }
        }

        // Debug name using string id registry if present
        std::string tex_name = ID_TO_STRING(texture_handle);
        SetD3DDebugName(texture->m_d3d_texture_2d, std::string("Tex2D: ") + tex_name);
        SetD3D12DebugName(texture->GetD3D12Resource(), StringUtil::StringToWString(std::string("Tex2D: ") + tex_name));

        DestroyTextureByID(texture_handle);
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

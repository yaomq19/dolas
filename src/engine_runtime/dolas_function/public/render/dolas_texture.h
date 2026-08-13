#ifndef DOLAS_TEXTURE_H
#define DOLAS_TEXTURE_H

#include <cstdint>
#include <string>
#include <d3d12.h>
#include "dolas_hash.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D12Resource;

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
        void SetD3D12Resource(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
        {
            m_d3d12_resource = resource;
            m_d3d12_resource_state = state;
        }
        void SetD3D12ResourceState(D3D12_RESOURCE_STATES state) { m_d3d12_resource_state = state; }
        void SetD3D12SrvHandles(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
        {
            m_d3d12_srv_cpu_handle = cpu_handle;
            m_d3d12_srv_gpu_handle = gpu_handle;
        }
        void SetD3D12RtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle) { m_d3d12_rtv_handle = rtv_handle; }
        void SetD3D12DsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle) { m_d3d12_dsv_handle = dsv_handle; }

        // Getters
        ID3D11Texture2D* GetD3DTexture2D() const { return m_d3d_texture_2d; }
        ID3D11ShaderResourceView* GetShaderResourceView();
        ID3D12Resource* GetD3D12Resource() const { return m_d3d12_resource; }
        D3D12_RESOURCE_STATES GetD3D12ResourceState() const { return m_d3d12_resource_state; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12SrvCpuHandle() const { return m_d3d12_srv_cpu_handle; }
        D3D12_GPU_DESCRIPTOR_HANDLE GetD3D12SrvGpuHandle() const { return m_d3d12_srv_gpu_handle; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12RtvHandle() const { return m_d3d12_rtv_handle; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12DsvHandle() const { return m_d3d12_dsv_handle; }
        bool HasD3D12Srv() const { return m_d3d12_srv_cpu_handle.ptr != 0 && m_d3d12_srv_gpu_handle.ptr != 0; }
        bool HasD3D12Rtv() const { return m_d3d12_rtv_handle.ptr != 0; }
        bool HasD3D12Dsv() const { return m_d3d12_dsv_handle.ptr != 0; }
        
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
        ID3D12Resource* m_d3d12_resource = nullptr;
        D3D12_RESOURCE_STATES m_d3d12_resource_state = D3D12_RESOURCE_STATE_COMMON;
        D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12_srv_cpu_handle {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_d3d12_srv_gpu_handle {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12_rtv_handle {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12_dsv_handle {};

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_mip_levels = 1;
    }; // class Texture
} // namespace Dolas

#endif // DOLAS_TEXTURE_H

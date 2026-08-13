#include "render/dolas_texture.h"
#include "dolas_engine.h"
#include "render/dolas_rhi.h"
#include <d3d11.h>
#include <iostream>
// #include <DirectXTex.h>

namespace Dolas
{
    Texture::Texture()
    {
        m_texture_type = DolasTextureType::TEXTURE_2D;
        m_texture_format = DolasTextureFormat::R8G8B8A8_UNORM;
    }

    Texture::~Texture()
    {
        Release();
    }

    void Texture::Release()
    {
        if (m_d3d_texture_2d)
        {
            m_d3d_texture_2d->Release();
            m_d3d_texture_2d = nullptr;
        }
        
        if (m_d3d_shader_resource_view)
        {
            m_d3d_shader_resource_view->Release();
            m_d3d_shader_resource_view = nullptr;
        }

        if (m_d3d12_resource)
        {
            m_d3d12_resource->Release();
            m_d3d12_resource = nullptr;
        }

        m_d3d12_resource_state = D3D12_RESOURCE_STATE_COMMON;
        m_d3d12_srv_cpu_handle = {};
        m_d3d12_srv_gpu_handle = {};
        m_d3d12_rtv_handle = {};
        m_d3d12_dsv_handle = {};
    }

    ID3D11ShaderResourceView* Texture::GetShaderResourceView()
    {
        if (m_d3d_shader_resource_view == nullptr && m_d3d_texture_2d)
        {
            m_d3d_shader_resource_view = g_dolas_engine.m_rhi->CreateShaderResourceView(m_d3d_texture_2d);
        }
        return m_d3d_shader_resource_view;
    }

} // namespace Dolas

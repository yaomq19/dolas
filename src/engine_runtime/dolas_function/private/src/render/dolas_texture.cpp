#include "render/dolas_texture.h"
#include "dolas_engine.h"
#include "render/dolas_rhi.h"
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
    }

    ID3D11ShaderResourceView* Texture::GetShaderResourceView()
    {
        if (m_d3d_shader_resource_view == nullptr)
        {
            m_d3d_shader_resource_view = g_dolas_engine.m_rhi->CreateShaderResourceView(m_d3d_texture_2d);
        }
        return m_d3d_shader_resource_view;
    }

} // namespace Dolas

#include "render/dolas_texture.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
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

} // namespace Dolas

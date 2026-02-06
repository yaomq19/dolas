#include "manager/dolas_render_resource_manager.h"
#include "dolas_engine.h"
#include "manager/dolas_texture_manager.h"
#include "render/dolas_rhi.h"
namespace Dolas
{
    RenderResourceManager::RenderResourceManager()
    {
    }
    
    RenderResourceManager::~RenderResourceManager()
    {
    }
    
    Bool RenderResourceManager::Initialize()
    {
        return true;
    }
    
    Bool RenderResourceManager::Clear()
    {
        m_render_resources.clear();
        return true;
    }
    
    RenderResource* RenderResourceManager::GetRenderResourceByID(RenderResourceID render_resource_id)
    {
        if (m_render_resources.find(render_resource_id) == m_render_resources.end())
        {
            return nullptr;
        }
        return m_render_resources[render_resource_id];
    }

    Bool RenderResourceManager::CreateRenderResourceByID(RenderResourceID render_resource_id)
    {
        TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
        DOLAS_RETURN_FALSE_IF_NULL(texture_manager);

        RenderResource* render_resource = DOLAS_NEW(RenderResource);

        DolasTexture2DDesc desc;

		TextureID gbuffer_a_texture_id = STRING_ID(gbuffer_a_map);
        desc.texture_handle = gbuffer_a_texture_id;
        desc.width = DEFAULT_CLIENT_WIDTH;
        desc.height = DEFAULT_CLIENT_HEIGHT;
        desc.format = DolasTextureFormat::R8G8B8A8_UNORM;
        desc.usage = DolasTextureUsage::RenderTarget;
        desc.generateMips = false;
        desc.arraySize = 1;
        if (texture_manager->DolasCreateTexture2D(desc))
        {
            render_resource->m_gbuffer_a_id = gbuffer_a_texture_id;
        }

        TextureID gbuffer_b_texture_id = STRING_ID(gbuffer_b_map);
        desc.texture_handle = gbuffer_b_texture_id;
        desc.width = DEFAULT_CLIENT_WIDTH;
        desc.height = DEFAULT_CLIENT_HEIGHT;
        desc.format = DolasTextureFormat::R8G8B8A8_UNORM;
        desc.usage = DolasTextureUsage::RenderTarget;
        desc.generateMips = false;
        desc.arraySize = 1;
        if (texture_manager->DolasCreateTexture2D(desc))
        {
            render_resource->m_gbuffer_b_id = gbuffer_b_texture_id;
        }

        TextureID gbuffer_c_texture_id = STRING_ID(gbuffer_c_map);
        desc.texture_handle = gbuffer_c_texture_id;
        desc.width = DEFAULT_CLIENT_WIDTH;
        desc.height = DEFAULT_CLIENT_HEIGHT;
        desc.format = DolasTextureFormat::R8G8B8A8_UNORM;
        desc.usage = DolasTextureUsage::RenderTarget;
        desc.generateMips = false;
        desc.arraySize = 1;
        if (texture_manager->DolasCreateTexture2D(desc))
        {
            render_resource->m_gbuffer_c_id = gbuffer_c_texture_id;
        }

		TextureID gbuffer_d_texture_id = STRING_ID(gbuffer_d_map);
		desc.texture_handle = gbuffer_d_texture_id;
		desc.width = DEFAULT_CLIENT_WIDTH;
		desc.height = DEFAULT_CLIENT_HEIGHT;
		desc.format = DolasTextureFormat::R8G8B8A8_UNORM;
		desc.usage = DolasTextureUsage::RenderTarget;
		desc.generateMips = false;
		desc.arraySize = 1;
		if (texture_manager->DolasCreateTexture2D(desc))
		{
			render_resource->m_gbuffer_d_id = gbuffer_d_texture_id;
		}

        TextureID depth_stencil_texture_id = STRING_ID(depth_stencil_map);
        desc.texture_handle = depth_stencil_texture_id;
        desc.width = DEFAULT_CLIENT_WIDTH;
        desc.height = DEFAULT_CLIENT_HEIGHT;
        desc.format = DolasTextureFormat::R24G8_TYPELESS;
        desc.usage = DolasTextureUsage::DepthStencil;
        desc.generateMips = false;
        desc.arraySize = 1;
        if (texture_manager->DolasCreateTexture2D(desc))
        {
            render_resource->m_depth_stencil_id = depth_stencil_texture_id;
        }

        TextureID scene_result_texture_id = STRING_ID(scene_result_map);
        desc.texture_handle = scene_result_texture_id;
        desc.width = DEFAULT_CLIENT_WIDTH;
        desc.height = DEFAULT_CLIENT_HEIGHT;
        desc.format = DolasTextureFormat::R8G8B8A8_UNORM;
        desc.usage = DolasTextureUsage::RenderTarget;
        desc.generateMips = false;
        desc.arraySize = 1;
        if (texture_manager->DolasCreateTexture2D(desc))
        {
            render_resource->m_scene_result_id = scene_result_texture_id;
        }

        m_render_resources[render_resource_id] = render_resource;
        return true;
    }
}
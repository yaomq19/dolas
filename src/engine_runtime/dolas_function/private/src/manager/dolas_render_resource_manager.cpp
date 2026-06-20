#include "manager/dolas_render_resource_manager.h"
#include "dolas_engine.h"
#include "dolas_log_system_manager.h"
#include "manager/dolas_texture_manager.h"
#include "render/dolas_rhi.h"
#include <vector>
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
        for (auto resource_iter = m_render_resources.begin(); resource_iter != m_render_resources.end(); ++resource_iter)
        {
            RenderResource* render_resource = resource_iter->second;
            DOLAS_DELETE(render_resource);
        }
        m_render_resources.clear();
        return true;
    }
    
    RenderResource* RenderResourceManager::GetRenderResourceByID(RenderResourceID render_resource_id)
    {
        auto it = m_render_resources.find(render_resource_id);
        return (it != m_render_resources.end()) ? it->second : nullptr;
    }

    Bool RenderResourceManager::CreateRenderResourceByID(RenderResourceID render_resource_id)
    {
        DOLAS_RETURN_FALSE_IF_FALSE(m_render_resources.find(render_resource_id) == m_render_resources.end());

        TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
        DOLAS_RETURN_FALSE_IF_NULL(texture_manager);

        RenderResource* render_resource = DOLAS_NEW(RenderResource);
        std::vector<TextureID> created_texture_ids;

        auto rollback_created_textures = [&]()
        {
            for (TextureID created_texture_id : created_texture_ids)
            {
                texture_manager->DestroyTextureByID(created_texture_id);
            }
            DOLAS_DELETE(render_resource);
        };

        auto create_required_texture = [&](TextureID texture_id, DolasTextureFormat texture_format, DolasTextureUsage texture_usage, TextureID& output_texture_id) -> Bool
        {
            DolasTexture2DDesc desc;
            desc.texture_handle = texture_id;
            desc.width = DEFAULT_CLIENT_WIDTH;
            desc.height = DEFAULT_CLIENT_HEIGHT;
            desc.format = texture_format;
            desc.usage = texture_usage;
            desc.generateMips = false;
            desc.arraySize = 1;

            if (!texture_manager->DolasCreateTexture2D(desc))
            {
                LOG_ERROR("RenderResourceManager::CreateRenderResourceByID: failed to create texture {0}", ID_TO_STRING(texture_id));
                return false;
            }

            output_texture_id = texture_id;
            created_texture_ids.push_back(texture_id);
            return true;
        };

		TextureID gbuffer_a_texture_id = STRING_ID(gbuffer_a_map);
        if (!create_required_texture(gbuffer_a_texture_id, DolasTextureFormat::R8G8B8A8_UNORM, DolasTextureUsage::RenderTarget, render_resource->m_gbuffer_a_id))
        {
            rollback_created_textures();
            return false;
        }

        TextureID gbuffer_b_texture_id = STRING_ID(gbuffer_b_map);
        if (!create_required_texture(gbuffer_b_texture_id, DolasTextureFormat::R8G8B8A8_UNORM, DolasTextureUsage::RenderTarget, render_resource->m_gbuffer_b_id))
        {
            rollback_created_textures();
            return false;
        }

        TextureID gbuffer_c_texture_id = STRING_ID(gbuffer_c_map);
        if (!create_required_texture(gbuffer_c_texture_id, DolasTextureFormat::R8G8B8A8_UNORM, DolasTextureUsage::RenderTarget, render_resource->m_gbuffer_c_id))
        {
            rollback_created_textures();
            return false;
        }

		TextureID gbuffer_d_texture_id = STRING_ID(gbuffer_d_map);
		if (!create_required_texture(gbuffer_d_texture_id, DolasTextureFormat::R8G8B8A8_UNORM, DolasTextureUsage::RenderTarget, render_resource->m_gbuffer_d_id))
		{
            rollback_created_textures();
            return false;
		}

        TextureID depth_stencil_texture_id = STRING_ID(depth_stencil_map);
        if (!create_required_texture(depth_stencil_texture_id, DolasTextureFormat::R24G8_TYPELESS, DolasTextureUsage::DepthStencil, render_resource->m_depth_stencil_id))
        {
            rollback_created_textures();
            return false;
        }

        TextureID scene_result_texture_id = STRING_ID(scene_result_map);
        if (!create_required_texture(scene_result_texture_id, DolasTextureFormat::R8G8B8A8_UNORM, DolasTextureUsage::RenderTarget, render_resource->m_scene_result_id))
        {
            rollback_created_textures();
            return false;
        }

        m_render_resources[render_resource_id] = render_resource;
        return true;
    }
}

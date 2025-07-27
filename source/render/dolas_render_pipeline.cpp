#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <cstddef>  // for offsetof

#include "base/dolas_paths.h"
#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_pipeline.h"
#include "render/dolas_render_entity.h"

#include "DirectXTex/DDSTextureLoader11.h"

namespace Dolas
{
    RenderPipeline::RenderPipeline()
    {

    }

    RenderPipeline::~RenderPipeline()
    {

    }

    bool RenderPipeline::Initialize()
    {
        return true;
    }

    bool RenderPipeline::Clear()
    {
        return true;
    }

    void RenderPipeline::Render(DolasRHI* rhi)
    {
        ClearPass(rhi);
        GBufferPass(rhi);
        DeferredShadingPass(rhi);
        ForwardShadingPass(rhi);
        PostProcessPass(rhi);
        PresentPass(rhi);
    }

    void RenderPipeline::ClearPass(DolasRHI* rhi)
    {
        const FLOAT clear_color[4] = {0.4f, 0.6f, 0.8f, 1.0f};
        rhi->m_d3d_immediate_context->ClearRenderTargetView(rhi->m_render_target_view, clear_color);
    }

    void RenderPipeline::GBufferPass(DolasRHI* rhi)
    {
        RenderEntityManager* render_entity_manager = g_dolas_engine.m_render_entity_manager;
        DOLAS_RETURN_IF_NULL(render_entity_manager);

		const std::string render_entity_name = "cube.entity";

        RenderEntity* render_entity = render_entity_manager->GetRenderEntityByFileName(render_entity_name);
        if (render_entity == nullptr)
        {
            RenderEntityID render_entity_id = render_entity_manager->CreateRenderEntity(render_entity_name);
            render_entity = render_entity_manager->GetRenderEntity(render_entity_id);
        }
        DOLAS_RETURN_IF_NULL(render_entity);

        // 设置 RT 和 视口
        /*rhi->m_d3d_immediate_context->OMSetRenderTargets(1, &rhi->m_render_target_view, rhi->m_depth_stencil_view);
        rhi->m_d3d_immediate_context->RSSetViewports(1, &rhi->m_viewport);*/
        render_entity->Draw(rhi);
    }

    void RenderPipeline::DeferredShadingPass(DolasRHI* rhi)
    {

    }

	void RenderPipeline::ForwardShadingPass(DolasRHI* rhi)
	{
        
	}
    
    void RenderPipeline::PostProcessPass(DolasRHI* rhi)
    {

    }

    void RenderPipeline::PresentPass(DolasRHI* rhi)
    {
        rhi->m_swap_chain->Present(0, 0);
    }
} // namespace Dolas

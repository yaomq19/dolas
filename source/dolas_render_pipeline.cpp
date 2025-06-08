#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <cstddef>  // for offsetof

#include "base/dolas_paths.h"
#include "base/dolas_string_utils.h"
#include "dolas_render_pipeline.h"
#include "DirectXTex/DDSTextureLoader11.h"
#include "dolas_render_entity.h"

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
		MeshRenderEntity* mesh_render_entity = new MeshRenderEntity();
        mesh_render_entity->Load("entities/cube.entity");
		mesh_render_entity->Draw(rhi);
		delete mesh_render_entity;
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

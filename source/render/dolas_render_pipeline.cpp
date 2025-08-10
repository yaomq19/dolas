#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <cstddef>  // for offsetof

#include "base/dolas_paths.h"
#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_entity_manager.h"
#include "manager/dolas_render_resource_manager.h"
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
        m_render_resource_id = STRING_ID("MainRenderResource");
        RenderResourceManager* render_resource_manager = g_dolas_engine.m_render_resource_manager;
        DOLAS_RETURN_FALSE_IF_NULL(render_resource_manager);
        Bool ret = render_resource_manager->CreateRenderResource(m_render_resource_id);
        DOLAS_RETURN_FALSE_IF_FALSE(ret);

        m_viewport.m_d3d_viewport.TopLeftX = 0.0f;
        m_viewport.m_d3d_viewport.TopLeftY = 0.0f;
        m_viewport.m_d3d_viewport.Width = DEFAULT_CLIENT_WIDTH;
        m_viewport.m_d3d_viewport.Height = DEFAULT_CLIENT_HEIGHT;
        m_viewport.m_d3d_viewport.MinDepth = 0.0f;
        m_viewport.m_d3d_viewport.MaxDepth = 1.0f;

        D3D11_RASTERIZER_DESC rasterizer_desc;
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_BACK;
        rasterizer_desc.FrontCounterClockwise = FALSE;
        rasterizer_desc.DepthBias = 0;
        rasterizer_desc.DepthBiasClamp = 0.0f;
        rasterizer_desc.SlopeScaledDepthBias = 0.0f;
        rasterizer_desc.DepthClipEnable = FALSE;
        rasterizer_desc.ScissorEnable = FALSE;
        rasterizer_desc.MultisampleEnable = FALSE;
        rasterizer_desc.AntialiasedLineEnable = FALSE;
        
        HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateRasterizerState(&rasterizer_desc, &m_rasterizer_state.m_d3d_rasterizer_state);
        if (FAILED(hr))
        {
            std::cout << "Failed to create rasterizer state!" << std::endl;
            return false;
        }

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
        rhi->m_d3d_immediate_context->ClearRenderTargetView(rhi->m_back_buffer_render_target_view, clear_color);
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
		RenderResource* render_resource = g_dolas_engine.m_render_resource_manager->GetRenderResource(m_render_resource_id);
        std::vector<RenderTargetView> rtvs(3);
        rtvs[0] = RenderTargetView(render_resource->m_gbuffer_a_id);
        rtvs[1] = RenderTargetView(render_resource->m_gbuffer_b_id);
        rtvs[2] = RenderTargetView(render_resource->m_gbuffer_c_id);
        DepthStencilView dsv(render_resource->m_depth_stencil_id);
        rhi->m_d3d_immediate_context->ClearDepthStencilView(dsv.GetD3DDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        rhi->SetRenderTargetView(3, rtvs, dsv);
        rhi->SetViewPort(m_viewport);
        rhi->SetRasterizerState(m_rasterizer_state);
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

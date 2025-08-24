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
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_texture_manager.h"
#include "DirectXTex/DDSTextureLoader11.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "manager/dolas_mesh_manager.h"

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
        rasterizer_desc.DepthClipEnable = TRUE;
        rasterizer_desc.ScissorEnable = FALSE;
        rasterizer_desc.MultisampleEnable = FALSE;
        rasterizer_desc.AntialiasedLineEnable = FALSE;
        
        HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateRasterizerState(&rasterizer_desc, &m_rasterizer_state.m_d3d_rasterizer_state);
        if (FAILED(hr))
        {
            std::cout << "Failed to create rasterizer state!" << std::endl;
            return false;
        }

        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;

        depth_stencil_desc.DepthEnable = TRUE;
        depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
        depth_stencil_desc.StencilEnable = FALSE;
        depth_stencil_desc.StencilReadMask = 0xFF;
        depth_stencil_desc.StencilWriteMask = 0xFF;

        hr = g_dolas_engine.m_rhi->m_d3d_device->CreateDepthStencilState(&depth_stencil_desc, &m_depth_stencil_state.m_d3d_depth_stencil_state);
        if (FAILED(hr))
        {
            std::cout << "Failed to create depth stencil state!" << std::endl;
            return false;
        }

        D3D11_BLEND_DESC blend_desc;
        blend_desc.AlphaToCoverageEnable = FALSE;
        blend_desc.IndependentBlendEnable = FALSE;
        blend_desc.RenderTarget[0].BlendEnable = FALSE;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = g_dolas_engine.m_rhi->m_d3d_device->CreateBlendState(&blend_desc, &m_blend_state.m_d3d_blend_state);
        if (FAILED(hr))
        {
            std::cout << "Failed to create blend state!" << std::endl;
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
        UserAnnotationScope scope(rhi, L"RenderPipeline");
        ClearPass(rhi);
        GBufferPass(rhi);
        DeferredShadingPass(rhi);
        ForwardShadingPass(rhi);
        PostProcessPass(rhi);
        PresentPass(rhi);
    }

    void RenderPipeline::ClearPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"ClearPass");
        rhi->BeginEvent(L"ClearBackRenderTarget");
        const FLOAT clear_color[4] = {0.4f, 0.6f, 0.8f, 1.0f};
        rhi->m_d3d_immediate_context->ClearRenderTargetView(rhi->m_back_buffer_render_target_view, clear_color);
        rhi->EndEvent();

        RenderResource* render_resource = g_dolas_engine.m_render_resource_manager->GetRenderResource(m_render_resource_id);
        DOLAS_RETURN_IF_NULL(render_resource);

        rhi->BeginEvent(L"ClearGBufferTextures");
        const FLOAT black_clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        rhi->m_d3d_immediate_context->ClearRenderTargetView(RenderTargetView(render_resource->m_gbuffer_a_id).GetD3DRenderTargetView(), black_clear_color);
        rhi->m_d3d_immediate_context->ClearRenderTargetView(RenderTargetView(render_resource->m_gbuffer_b_id).GetD3DRenderTargetView(), black_clear_color);
        rhi->m_d3d_immediate_context->ClearRenderTargetView(RenderTargetView(render_resource->m_gbuffer_c_id).GetD3DRenderTargetView(), black_clear_color);
        rhi->EndEvent();
        
        rhi->BeginEvent(L"ClearSceneDepthTexture");
        rhi->m_d3d_immediate_context->ClearDepthStencilView(DepthStencilView(render_resource->m_depth_stencil_id).GetD3DDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        rhi->EndEvent();

        rhi->BeginEvent(L"ClearSceneResultTexture");
        rhi->m_d3d_immediate_context->ClearRenderTargetView(RenderTargetView(render_resource->m_scene_result_id).GetD3DRenderTargetView(), black_clear_color);
        rhi->EndEvent();
    }

    void RenderPipeline::GBufferPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"GBufferPass");
        
        RenderEntityManager* render_entity_manager = g_dolas_engine.m_render_entity_manager;
        DOLAS_RETURN_IF_NULL(render_entity_manager);

		const std::string render_entity_name = "cube.entity";

        RenderEntity* render_entity = render_entity_manager->GetRenderEntityByFileName(render_entity_name);
        if (render_entity == nullptr)
        {
            RenderEntityID render_entity_id = render_entity_manager->CreateRenderEntityFromFile(render_entity_name);
            render_entity = render_entity_manager->GetRenderEntityByID(render_entity_id);
        }
        DOLAS_RETURN_IF_NULL(render_entity);

        // 设置 RT 和 视口
		RenderResource* render_resource = g_dolas_engine.m_render_resource_manager->GetRenderResource(m_render_resource_id);
        DOLAS_RETURN_IF_NULL(render_resource);

        std::vector<RenderTargetView> rtvs;
        rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_a_id));
        rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_b_id));
		rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_c_id));
		DepthStencilView dsv(render_resource->m_depth_stencil_id);

        rhi->SetRenderTargetView(rtvs, dsv);
        rhi->SetViewPort(m_viewport);
        rhi->SetRasterizerState(m_rasterizer_state);
        rhi->SetDepthStencilState(m_depth_stencil_state);
        rhi->SetBlendState(m_blend_state);
        render_entity->Draw(rhi);
    }

    void RenderPipeline::DeferredShadingPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"DeferredShadingPass");
        // RenderEntityManager* render_entity_manager = g_dolas_engine.m_render_entity_manager;
        // DOLAS_RETURN_IF_NULL(render_entity_manager);

		// const std::string render_entity_name = "quad.entity";

        // RenderEntity* render_entity = render_entity_manager->GetRenderEntityByFileName(render_entity_name);
        // if (render_entity == nullptr)
        // {
        //     RenderEntityID render_entity_id = render_entity_manager->CreateRenderEntityFromFile(render_entity_name);
        //     render_entity = render_entity_manager->GetRenderEntity(render_entity_id);
        // }
        // DOLAS_RETURN_IF_NULL(render_entity);

        // // 设置 RT 和 视口
		RenderResource* render_resource = g_dolas_engine.m_render_resource_manager->GetRenderResource(m_render_resource_id);
        DOLAS_RETURN_IF_NULL(render_resource);

        std::vector<RenderTargetView> rtvs;
		rtvs.push_back(RenderTargetView(render_resource->m_scene_result_id));
        // DepthStencilView dsv(render_resource->m_depth_stencil_id);

        rhi->SetRenderTargetView(rtvs);
        rhi->SetViewPort(m_viewport);
        rhi->SetRasterizerState(m_rasterizer_state);
        rhi->SetDepthStencilState(m_depth_stencil_state);
        rhi->SetBlendState(m_blend_state);

		RenderEntityManager* entity_manager = g_dolas_engine.m_render_entity_manager;
        RenderEntityID render_entity_id = entity_manager->CreateRenderEntity(STRING_ID(Quad));
		RenderEntity* render_entity = entity_manager->GetRenderEntityByID(render_entity_id);
        

        MeshID quad_mesh_id = g_dolas_engine.m_mesh_manager->GetQuadMeshID();
        render_entity->SetMeshID(quad_mesh_id);

        MaterialID material_id = g_dolas_engine.m_material_manager->GetDeferredShadingMaterialID();
        render_entity->SetMaterialID(material_id);

        render_entity->Draw(rhi);
    }

	void RenderPipeline::ForwardShadingPass(DolasRHI* rhi)
	{
        UserAnnotationScope scope(rhi, L"ForwardShadingPass");
        
	}
    
    void RenderPipeline::PostProcessPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"PostProcessPass");

    }

    void RenderPipeline::PresentPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"PresentPass");
		RenderResource* render_resource = g_dolas_engine.m_render_resource_manager->GetRenderResource(m_render_resource_id);
        if (render_resource)
        {
            Texture* scene_result_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(render_resource->m_scene_result_id);
			DOLAS_RETURN_IF_NULL(scene_result_texture);
			rhi->m_d3d_immediate_context->CopyResource(rhi->m_swap_chain_back_texture, scene_result_texture->GetD3DTexture2D());
		}
		else
		{
			std::cout << "Render resource not found!" << std::endl;
        }
        rhi->m_swap_chain->Present(0, 0);
    }
} // namespace Dolas

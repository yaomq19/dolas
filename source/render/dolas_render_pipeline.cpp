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
#include "DirectXTex.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "manager/dolas_mesh_manager.h"
#include "render/dolas_shader.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
#include "render/dolas_render_scene.h"
#include "manager/dolas_render_camera_manager.h"
#include "manager/dolas_render_scene_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_geometry_manager.h"
#include "render/dolas_render_camera.h"
#include "manager/dolas_tick_manager.h"
#include "manager/dolas_imgui_manager.h"
#include "manager/dolas_debug_draw_manager.h"
namespace Dolas
{
	const UInt k_print_debug_info_every_n_frames = 20;

    RenderPipeline::RenderPipeline() : m_viewport(0.0f, 0.0f, DEFAULT_CLIENT_WIDTH, DEFAULT_CLIENT_HEIGHT, 0.0f, 1.0f)
    {

    }

    RenderPipeline::~RenderPipeline()
    {

    }

    bool RenderPipeline::Initialize()
    {
        g_dolas_engine.m_rhi->VSSetConstantBuffers();
        g_dolas_engine.m_rhi->PSSetConstantBuffers();
        return true;
    }

    bool RenderPipeline::Clear()
    {
        return true;
    }

    void RenderPipeline::Render(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"RenderPipeline");
        rhi->UpdatePerFrameParameters();
		RenderCamera* render_camera = TryGetRenderCamera();
		DOLAS_RETURN_IF_NULL(render_camera);
		rhi->UpdatePerViewParameters(render_camera);
        ClearPass(rhi);
        GBufferPass(rhi);
        DeferredShadingPass(rhi);
        ForwardShadingPass(rhi);
        SkyboxPass(rhi);
        PostProcessPass(rhi);
        DebugPass(rhi);
        PresentPass(rhi);
    }

    void RenderPipeline::SetRenderViewID(RenderViewID id)
    {
        m_render_view_id = id;
    }

    void RenderPipeline::ClearPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"ClearPass");
		RenderResource* render_resource = TryGetRenderResource();
		DOLAS_RETURN_IF_NULL(render_resource);

		const FLOAT black_clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        rhi->BeginEvent(L"ClearBackRenderTarget");
        rhi->ClearRenderTargetView(rhi->GetBackBufferRTV(), black_clear_color);
        rhi->EndEvent();

        rhi->BeginEvent(L"ClearGBufferTextures");
        auto gbuffer_a_rtv = rhi->CreateRenderTargetView(render_resource->m_gbuffer_a_id);
        auto gbuffer_b_rtv = rhi->CreateRenderTargetView(render_resource->m_gbuffer_b_id);
        auto gbuffer_c_rtv = rhi->CreateRenderTargetView(render_resource->m_gbuffer_c_id);
        rhi->ClearRenderTargetView(gbuffer_a_rtv, black_clear_color);
        rhi->ClearRenderTargetView(gbuffer_b_rtv, black_clear_color);
        rhi->ClearRenderTargetView(gbuffer_c_rtv, black_clear_color);
        rhi->EndEvent();
        
        rhi->BeginEvent(L"ClearSceneDepthTexture");
        std::shared_ptr<DepthStencilView> scene_dsv = rhi->CreateDepthStencilView(render_resource->m_depth_stencil_id);
		DepthClearParams depth_clear_params;
        depth_clear_params.enable = true;
        depth_clear_params.clear_value = 1.0f;
		StencilClearParams stencil_clear_params;
		stencil_clear_params.enable = true;
		stencil_clear_params.clear_value = StencilMaskEnum_SKY;
        rhi->ClearDepthStencilView(scene_dsv, depth_clear_params, stencil_clear_params);
        rhi->EndEvent();

        rhi->BeginEvent(L"ClearSceneResultTexture");
		auto scene_result_rtv = rhi->CreateRenderTargetView(render_resource->m_scene_result_id);
        rhi->ClearRenderTargetView(scene_result_rtv, black_clear_color);
        rhi->EndEvent();
    }

    void RenderPipeline::GBufferPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"GBufferPass");
        
        RenderScene* render_scene = TryGetRenderScene();
        DOLAS_RETURN_IF_NULL(render_scene);

        // 设置 RT 和 视口
		RenderResource* render_resource = TryGetRenderResource();
        DOLAS_RETURN_IF_NULL(render_resource);

        std::vector<std::shared_ptr<RenderTargetView>> rtvs;
        rtvs.push_back(g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_gbuffer_a_id));
        rtvs.push_back(g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_gbuffer_b_id));
        rtvs.push_back(g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_gbuffer_c_id));

        auto dsv = g_dolas_engine.m_rhi->CreateDepthStencilView(render_resource->m_depth_stencil_id);

        rhi->SetRenderTargetViewAndDepthStencilView(rtvs, dsv);
        rhi->SetViewPort(m_viewport);

        rhi->SetRasterizerState(RasterizerStateType_SolidBackCull);
        rhi->SetDepthStencilState(DepthStencilStateType_DepthWriteLess_StencilWriteStatic);
        rhi->SetBlendState(BlendStateType_Opaque);

        const std::vector<RenderEntityID>& render_entities = render_scene->GetRenderEntities();
        for (RenderEntityID render_entity_id: render_entities)
        {
            RenderEntity* render_entity = g_dolas_engine.m_render_entity_manager->GetRenderEntityByID(render_entity_id);
			DOLAS_CONTINUE_IF_NULL(render_entity);
			render_entity->Draw(rhi);
        }
    }

    void RenderPipeline::DeferredShadingPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"DeferredShadingPass");

        RenderResource* render_resource = TryGetRenderResource();
        DOLAS_RETURN_IF_NULL(render_resource);

        std::vector<std::shared_ptr<RenderTargetView>> rtvs;
        auto scene_result_rtv = g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_scene_result_id);
        rtvs.push_back(scene_result_rtv);

        rhi->SetRenderTargetViewWithoutDepthStencilView(rtvs);
        rhi->SetViewPort(m_viewport);

        rhi->SetRasterizerState(RasterizerStateType_SolidNoneCull);
        rhi->SetDepthStencilState(DepthStencilStateType_DepthDisabled_StencilDisable);
        rhi->SetBlendState(BlendStateType_Opaque);

        Material* material = g_dolas_engine.m_material_manager->GetDeferredShadingMaterial();
        DOLAS_RETURN_IF_NULL(material);

        VertexContext* vertex_context = material->GetVertexShader();
        DOLAS_RETURN_IF_NULL(vertex_context);

        PixelContext* pixel_context = material->GetPixelShader();
        DOLAS_RETURN_IF_NULL(pixel_context);

        pixel_context->SetShaderResourceView(0, render_resource->m_gbuffer_a_id);
        pixel_context->SetShaderResourceView(1, render_resource->m_gbuffer_b_id);
        pixel_context->SetShaderResourceView(2, render_resource->m_gbuffer_c_id);

        if (rhi->BindVertexContext(vertex_context, material->GetVertexShaderTextures())
            && rhi->BindPixelContext(pixel_context, material->GetPixelShaderTextures()))
        {
            RenderPrimitiveID quad_render_primitive_id = g_dolas_engine.m_geometry_manager->GetGeometryRenderPrimitiveID(BaseGeometryType::_QUAD);
            rhi->DrawRenderPrimitive(quad_render_primitive_id);
        }
    }

	void RenderPipeline::ForwardShadingPass(DolasRHI* rhi)
	{
        UserAnnotationScope scope(rhi, L"ForwardShadingPass");
	}
    
    void RenderPipeline::SkyboxPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"SkyboxPass");
        // TODO: Implement SkyboxPass
		RenderResource* render_resource = TryGetRenderResource();
		DOLAS_RETURN_IF_NULL(render_resource);

		std::vector<std::shared_ptr<RenderTargetView>> rtvs;
		rtvs.push_back(g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_scene_result_id));
		std::shared_ptr<DepthStencilView> dsv = g_dolas_engine.m_rhi->CreateDepthStencilView(render_resource->m_depth_stencil_id);
		rhi->SetRenderTargetViewAndDepthStencilView(rtvs, dsv);
		rhi->SetViewPort(m_viewport);
		rhi->SetRasterizerState(RasterizerStateType_SolidNoneCull);
		rhi->SetDepthStencilState(DepthStencilStateType_DepthDisabled_StencilReadSky);
		rhi->SetBlendState(BlendStateType_Opaque);

        RenderCamera* eye_camera = TryGetRenderCamera();
		DOLAS_RETURN_IF_NULL(eye_camera);
        
        const Float hack_scale = 0.99f;
        Float scale = eye_camera->GetFarPlane() * hack_scale;
        Pose pose(eye_camera->GetPosition(), Quaternion(1.0, 0.0, 0.0, 0.0), Vector3(scale, scale, scale));
        rhi->UpdatePerObjectParameters(pose);

        Material* material = g_dolas_engine.m_material_manager->GetSkyBoxMaterial();
        DOLAS_RETURN_IF_NULL(material);

        VertexContext* vertex_context = material->GetVertexShader();
        DOLAS_RETURN_IF_NULL(vertex_context);

        PixelContext* pixel_context = material->GetPixelShader();
        DOLAS_RETURN_IF_NULL(pixel_context);

        pixel_context->SetShaderResourceView(0, g_dolas_engine.m_texture_manager->GetGlobalTexture(GlobalTextureType::GLOBAL_TEXTURE_SKY_BOX));

		// 绑定 Shader
        if (rhi->BindVertexContext(vertex_context, material->GetVertexShaderTextures())
            && rhi->BindPixelContext(pixel_context, material->GetPixelShaderTextures()))
        {
            RenderPrimitiveID sphere_render_primitive_id = g_dolas_engine.m_geometry_manager->GetGeometryRenderPrimitiveID(BaseGeometryType::_SPHERE);
            rhi->DrawRenderPrimitive(sphere_render_primitive_id);
        }
    }

    void RenderPipeline::PostProcessPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"PostProcessPass");
    }

    void RenderPipeline::DebugPass(DolasRHI* rhi)
    {
        g_dolas_engine.m_debug_draw_manager->Render();
        g_dolas_engine.m_imgui_manager->Render();
    }

    void RenderPipeline::PresentPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"PresentPass");
		RenderResource* render_resource = TryGetRenderResource();
        DOLAS_RETURN_IF_NULL(render_resource);
        rhi->Present(render_resource->m_scene_result_id);
    }

    RenderScene* RenderPipeline::TryGetRenderScene() const
    {
		RenderViewManager* render_view_manager = g_dolas_engine.m_render_view_manager;
		DOLAS_RETURN_NULL_IF_NULL(render_view_manager);
		RenderView* render_view = render_view_manager->GetRenderView(m_render_view_id);
		DOLAS_RETURN_NULL_IF_NULL(render_view);
		RenderSceneID render_scene_id = render_view->GetRenderSceneID();

		RenderSceneManager* render_scene_manager = g_dolas_engine.m_render_scene_manager;
		DOLAS_RETURN_NULL_IF_NULL(render_scene_manager);

        return render_scene_manager->GetRenderSceneByID(render_scene_id);
    }

    RenderResource* RenderPipeline::TryGetRenderResource() const
    {
        RenderViewManager* render_view_manager = g_dolas_engine.m_render_view_manager;
        DOLAS_RETURN_NULL_IF_NULL(render_view_manager);
        RenderView* render_view = render_view_manager->GetRenderView(m_render_view_id);
        DOLAS_RETURN_NULL_IF_NULL(render_view);

        RenderResourceManager* render_resource_manager = g_dolas_engine.m_render_resource_manager;
        DOLAS_RETURN_NULL_IF_NULL(render_resource_manager);
        RenderResourceID render_resource_id = render_view->GetRenderResourceID();
        return render_resource_manager->GetRenderResourceByID(render_resource_id);
    }

    class RenderCamera* RenderPipeline::TryGetRenderCamera() const
    {
        RenderViewManager* render_view_manager = g_dolas_engine.m_render_view_manager;
        DOLAS_RETURN_NULL_IF_NULL(render_view_manager);
        RenderView* render_view = render_view_manager->GetRenderView(m_render_view_id);
        DOLAS_RETURN_NULL_IF_NULL(render_view);

        RenderCameraManager* render_camera_manager = g_dolas_engine.m_render_camera_manager;
        DOLAS_RETURN_NULL_IF_NULL(render_camera_manager);
        RenderCameraID render_camera_id = render_view->GetRenderCameraID();
        return render_camera_manager->GetRenderCameraByID(render_camera_id);
    }
} // namespace Dolas

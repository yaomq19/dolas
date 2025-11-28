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
		stencil_clear_params.clear_value = 0;
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
        
        RenderSceneManager* render_scene_manager = g_dolas_engine.m_render_scene_manager;
		DOLAS_RETURN_IF_NULL(render_scene_manager);

        RenderScene* render_scene = TryGetRenderScene();

		/*const std::string render_entity_name = "quad.entity";
        RenderEntity* render_entity = render_entity_manager->GetRenderEntityByFileName(render_entity_name);
        if (render_entity == nullptr)
        {
            RenderEntityID render_entity_id = render_entity_manager->CreateRenderEntityFromFile(render_entity_name);
            render_entity = render_entity_manager->GetRenderEntityByID(render_entity_id);
        }
        DOLAS_RETURN_IF_NULL(render_entity);*/

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

        rhi->SetRasterizerState(RasterizerStateType::SolidBackCull);
        rhi->SetDepthStencilState(DepthStencilStateType::DepthEnabled);
        rhi->SetBlendState(BlendStateType::Opaque);

        // render_entity->Draw(rhi);
    }

    void RenderPipeline::DeferredShadingPass(DolasRHI* rhi)
    {
        UserAnnotationScope scope(rhi, L"DeferredShadingPass");

        RenderResource* render_resource = TryGetRenderResource();
        DOLAS_RETURN_IF_NULL(render_resource);

        Texture* gbuffer_a_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(render_resource->m_gbuffer_a_id);
        Texture* gbuffer_b_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(render_resource->m_gbuffer_b_id);
        Texture* gbuffer_c_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(render_resource->m_gbuffer_c_id);
        DOLAS_RETURN_IF_NULL(gbuffer_a_texture);
        DOLAS_RETURN_IF_NULL(gbuffer_b_texture);
        DOLAS_RETURN_IF_NULL(gbuffer_c_texture);

        std::vector<std::shared_ptr<RenderTargetView>> rtvs;
        auto scene_result_rtv = g_dolas_engine.m_rhi->CreateRenderTargetView(render_resource->m_scene_result_id);
        rtvs.push_back(scene_result_rtv);

        rhi->SetRenderTargetViewWithoutDepthStencilView(rtvs);
        rhi->SetViewPort(m_viewport);

        rhi->SetRasterizerState(RasterizerStateType::SolidNoneCull);
        rhi->SetDepthStencilState(DepthStencilStateType::DepthDisabled);
        rhi->SetBlendState(BlendStateType::Opaque);

		RenderEntityManager* entity_manager = g_dolas_engine.m_render_entity_manager;
        RenderEntityID quad_render_entity_id = STRING_ID(Quad);
		RenderEntity* render_entity = entity_manager->GetRenderEntityByID(quad_render_entity_id);
        if (render_entity == nullptr)
        {
            entity_manager->CreateRenderEntity(quad_render_entity_id);
            render_entity = entity_manager->GetRenderEntityByID(quad_render_entity_id);
        }
        
        MeshID quad_mesh_id = g_dolas_engine.m_mesh_manager->GetQuadMeshID();
        render_entity->SetMeshID(quad_mesh_id);

        Material* material = g_dolas_engine.m_material_manager->GetDeferredShadingMaterial();

        PixelShader* pixel_shader = material->GetPixelShader();
        DOLAS_RETURN_IF_NULL(pixel_shader);

        pixel_shader->ClearShaderResourceViews();

        pixel_shader->SetShaderResourceView(0, gbuffer_a_texture->GetShaderResourceView());
        pixel_shader->SetShaderResourceView(1, gbuffer_b_texture->GetShaderResourceView());
        pixel_shader->SetShaderResourceView(2, gbuffer_c_texture->GetShaderResourceView());

        render_entity->Draw(rhi);
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

		rhi->SetRenderTargetViewWithoutDepthStencilView(rtvs);
		rhi->SetViewPort(m_viewport);

		rhi->SetRasterizerState(RasterizerStateType::SolidNoneCull);
		rhi->SetDepthStencilState(DepthStencilStateType::DepthDisabled);
		rhi->SetBlendState(BlendStateType::Opaque);

        

        
        Pose pose;
        RenderCamera* eye_camera = TryGetRenderCamera();
		DOLAS_RETURN_IF_NULL(eye_camera);
        
        pose.m_postion = eye_camera->GetPosition();
		pose.m_rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
        const Float hack_scale = 0.99f;
		Float scale = eye_camera->GetFarPlane() * hack_scale;
		pose.m_scale = Vector3(scale, scale, scale);
        rhi->UpdatePerObjectParameters(pose);

        Texture* sky_box_texture = g_dolas_engine.m_texture_manager->GetGlobalTexture(GlobalTextureType::GLOBAL_TEXTURE_SKY_BOX);
		DOLAS_RETURN_IF_NULL(sky_box_texture);

        Material* material = g_dolas_engine.m_material_manager->GetSkyBoxMaterial();
        DOLAS_RETURN_IF_NULL(material);

        VertexShader* vertex_shader = material->GetVertexShader();
        DOLAS_RETURN_IF_NULL(vertex_shader);

        PixelShader* pixel_shader = material->GetPixelShader();
        DOLAS_RETURN_IF_NULL(pixel_shader);

        pixel_shader->ClearShaderResourceViews();
        pixel_shader->SetShaderResourceView(0, sky_box_texture->GetShaderResourceView());

        // 处理材质球自带纹理
		material->BindVertexShaderTextures();
		material->BindPixelShaderTextures();
		// 绑定 Shader
        material->GetVertexShader()->Bind(rhi, nullptr, 0);
        material->GetPixelShader()->Bind(rhi, nullptr, 0);
        
        rhi->VSSetConstantBuffers();
        rhi->PSSetConstantBuffers();

        RenderPrimitiveID sphere_render_primitive_id = g_dolas_engine.m_geometry_manager->GetGeometryRenderPrimitiveID(BaseGeometryType::_SPHERE);
        rhi->DrawRenderPrimitive(sphere_render_primitive_id, vertex_shader->GetD3DShaderBlob());
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

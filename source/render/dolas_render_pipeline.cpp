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
#include "manager/dolas_render_state_manager.h"
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

    RenderPipeline::RenderPipeline()
    {

    }

    RenderPipeline::~RenderPipeline()
    {

    }

    bool RenderPipeline::Initialize()
    {


        m_viewport.m_d3d_viewport.TopLeftX = 0.0f;
        m_viewport.m_d3d_viewport.TopLeftY = 0.0f;
        m_viewport.m_d3d_viewport.Width = DEFAULT_CLIENT_WIDTH;
        m_viewport.m_d3d_viewport.Height = DEFAULT_CLIENT_HEIGHT;
        m_viewport.m_d3d_viewport.MinDepth = 0.0f;
        m_viewport.m_d3d_viewport.MaxDepth = 1.0f;

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
        rhi->BeginEvent(L"ClearBackRenderTarget");
        const FLOAT clear_color[4] = {0.4f, 0.6f, 0.8f, 1.0f};
        rhi->m_d3d_immediate_context->ClearRenderTargetView(rhi->m_back_buffer_render_target_view, clear_color);
        rhi->EndEvent();

        RenderResource* render_resource = TryGetRenderResource();
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

        std::vector<RenderTargetView> rtvs;
        rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_a_id));
        rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_b_id));
		rtvs.push_back(RenderTargetView(render_resource->m_gbuffer_c_id));
		DepthStencilView dsv(render_resource->m_depth_stencil_id);

        rhi->SetRenderTargetView(rtvs, dsv);
        rhi->SetViewPort(m_viewport);

        RasterizerState* rasterizer_state = g_dolas_engine.m_render_state_manager->GetRasterizerState(RasterizerStateType::SolidBackCull);
        DepthStencilState* depth_stencil_state = g_dolas_engine.m_render_state_manager->GetDepthStencilState(DepthStencilStateType::DepthEnabled);
		BlendState* blend_state = g_dolas_engine.m_render_state_manager->GetBlendState(BlendStateType::Opaque);

        rhi->SetRasterizerState(*rasterizer_state);
        rhi->SetDepthStencilState(*depth_stencil_state);
        rhi->SetBlendState(*blend_state);

        
        
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

        std::vector<RenderTargetView> rtvs;
		rtvs.push_back(RenderTargetView(render_resource->m_scene_result_id));

        rhi->SetRenderTargetView(rtvs);
        rhi->SetViewPort(m_viewport);

        DepthStencilState* depth_stencil_state = g_dolas_engine.m_render_state_manager->GetDepthStencilState(DepthStencilStateType::DepthDisabled);
        RasterizerState* rasterizer_state = g_dolas_engine.m_render_state_manager->GetRasterizerState(RasterizerStateType::SolidNoneCull);
        BlendState* blend_state = g_dolas_engine.m_render_state_manager->GetBlendState(BlendStateType::Opaque);

        rhi->SetRasterizerState(*rasterizer_state);
        rhi->SetDepthStencilState(*depth_stencil_state);
        rhi->SetBlendState(*blend_state);

		

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

        MaterialID material_id = g_dolas_engine.m_material_manager->GetDeferredShadingMaterialID();
        render_entity->SetMaterialID(material_id);

        Material* material = g_dolas_engine.m_material_manager->GetMaterial(material_id);
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

		std::vector<RenderTargetView> rtvs;
		rtvs.push_back(RenderTargetView(render_resource->m_scene_result_id));

		rhi->SetRenderTargetView(rtvs);
		rhi->SetViewPort(m_viewport);

		DepthStencilState* depth_stencil_state = g_dolas_engine.m_render_state_manager->GetDepthStencilState(DepthStencilStateType::DepthDisabled);
		RasterizerState* rasterizer_state = g_dolas_engine.m_render_state_manager->GetRasterizerState(RasterizerStateType::SolidNoneCull);
		BlendState* blend_state = g_dolas_engine.m_render_state_manager->GetBlendState(BlendStateType::Opaque);

		rhi->SetRasterizerState(*rasterizer_state);
		rhi->SetDepthStencilState(*depth_stencil_state);
		rhi->SetBlendState(*blend_state);

        RenderPrimitiveID sphere_render_primitive_id = g_dolas_engine.m_geometry_manager->GetGeometryRenderPrimitiveID(BaseGeometryType::_SPHERE);
		
        RenderPrimitiveManager* primitive_manager = g_dolas_engine.m_render_primitive_manager;
		RenderPrimitive* sphere_render_primitive = primitive_manager->GetRenderPrimitiveByID(sphere_render_primitive_id);
		DOLAS_RETURN_IF_NULL(sphere_render_primitive);
        
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

        MaterialID material_id = g_dolas_engine.m_material_manager->GetSkyBoxMaterialID();
        Material* material = g_dolas_engine.m_material_manager->GetMaterial(material_id);
        DOLAS_RETURN_IF_NULL(material);
        // 处理纹理
		material->BindVertexShaderTextures();
		material->BindPixelShaderTextures();
		// 绑定 Shader
        material->GetVertexShader()->Bind(rhi, nullptr, 0);
        material->GetPixelShader()->Bind(rhi, nullptr, 0);
        
        rhi->VSSetConstantBuffers();
        rhi->PSSetConstantBuffers();

		PixelShader* pixel_shader = material->GetPixelShader();
		DOLAS_RETURN_IF_NULL(pixel_shader);

		pixel_shader->ClearShaderResourceViews();
		pixel_shader->SetShaderResourceView(0, sky_box_texture->GetShaderResourceView());

        ID3D11InputLayout* input_layout = nullptr; 
        std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;
        if (sphere_render_primitive->m_input_layout_type == InputLayoutType::POS_3_UV_2_NORM_3)
        {
            input_layout_desc = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
        }
        else if (sphere_render_primitive->m_input_layout_type == InputLayoutType::POS_3_UV_2)
        {
            input_layout_desc = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
        }
        else if (sphere_render_primitive->m_input_layout_type == InputLayoutType::POS_3)
        {
            input_layout_desc = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
        }
        else
        {
            return;
        }

        g_dolas_engine.m_rhi->m_d3d_device->CreateInputLayout(
            input_layout_desc.data(),
            input_layout_desc.size(),
            material->GetVertexShader()->GetD3DShaderBlob()->GetBufferPointer(),
            material->GetVertexShader()->GetD3DShaderBlob()->GetBufferSize(),
            &input_layout);
        rhi->m_d3d_immediate_context->IASetInputLayout(input_layout);
        input_layout->Release();
        input_layout = nullptr;

        sphere_render_primitive->Draw(rhi);
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
        if (render_resource)
        {
            Texture* scene_result_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(render_resource->m_scene_result_id);
			DOLAS_RETURN_IF_NULL(scene_result_texture);
			rhi->m_d3d_immediate_context->CopyResource(rhi->m_swap_chain_back_texture, scene_result_texture->GetD3DTexture2D());
		}
		else
		{
			LOG_ERROR("Render resource not found!");
        }
        rhi->m_swap_chain->Present(0, 0);
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

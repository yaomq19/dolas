#include <fstream>
#include <iostream>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include "base/dolas_paths.h"
#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_mesh.h"
#include "render/dolas_material.h"
#include "base/dolas_dx_trace.h"
#include "render/dolas_shader.h"
#include "manager/dolas_render_primitive_manager.h"
#include "render/dolas_render_primitive.h"
namespace Dolas
{
    RenderEntity::RenderEntity()
    {

    }

    RenderEntity::~RenderEntity()
    {

    }

    bool RenderEntity::Clear()
    {
        return true;
    }

    void RenderEntity::Draw(DolasRHI* rhi, Transform transform)
    {
		Mesh* mesh = g_dolas_engine.m_mesh_manager->GetMesh(m_mesh_id);
		DOLAS_RETURN_IF_NULL(mesh);
		Material* material = g_dolas_engine.m_material_manager->GetMaterial(m_material_id);
		DOLAS_RETURN_IF_NULL(material);

        // 处理纹理
		material->BindVertexShaderTextures();
		material->BindPixelShaderTextures();
		// 绑定 Shader
        material->GetVertexShader()->Bind(rhi, nullptr, 0);
        material->GetPixelShader()->Bind(rhi, nullptr, 0);
        
        rhi->VSSetConstantBuffers();
        rhi->PSSetConstantBuffers();

        RenderPrimitiveID render_primitive_id = mesh->GetRenderPrimitiveID();
        RenderPrimitive* render_primitive = g_dolas_engine.m_render_primitive_manager->GetRenderPrimitiveByID(render_primitive_id);

        ID3D11InputLayout* input_layout = nullptr; 
        std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_desc;
        if (render_primitive->m_input_layout_type == DolasInputLayoutType::POS_3_UV_2_NORM_3)
        {
            input_layout_desc = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
        }
        else if (render_primitive->m_input_layout_type == DolasInputLayoutType::POS_3_UV_2)
        {
            input_layout_desc = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

        // 发起DC
        if (render_primitive)
        {
			render_primitive->Draw(rhi);
        }
    }

    void RenderEntity::SetMeshID(MeshID mesh_id)
    {
        m_mesh_id = mesh_id;
    }

    void RenderEntity::SetMaterialID(MaterialID material_id)
    {
        m_material_id = material_id;
    }
}
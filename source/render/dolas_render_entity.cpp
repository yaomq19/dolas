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

    void RenderEntity::Draw(DolasRHI* rhi)
    {
		Mesh* mesh = g_dolas_engine.m_mesh_manager->GetMesh(m_mesh_id);
		DOLAS_RETURN_IF_NULL(mesh);
		Material* material = g_dolas_engine.m_material_manager->GetMaterialByID(m_material_id);
		DOLAS_RETURN_IF_NULL(material);

        VertexShader* vertex_shader = material->GetVertexShader();
        DOLAS_RETURN_IF_NULL(vertex_shader);

        PixelShader* pixel_shader = material->GetPixelShader();
        DOLAS_RETURN_IF_NULL(pixel_shader);

		// 绑定 Shader
        vertex_shader->Bind(rhi, nullptr, 0, material->GetVertexShaderTextures());
        pixel_shader->Bind(rhi, nullptr, 0, material->GetPixelShaderTextures());
        
        RenderPrimitiveID render_primitive_id = mesh->GetRenderPrimitiveID();

        g_dolas_engine.m_rhi->DrawRenderPrimitive(render_primitive_id, nullptr);
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
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
        rhi->UpdatePerObjectParameters(m_pose);

        for (const auto& component : m_components)
        {
            Material* material = g_dolas_engine.m_material_manager->GetMaterialByID(component.m_material_id);
            if (!material) continue;

            std::shared_ptr<VertexContext> vertex_context = material->GetVertexContext();
            if (!vertex_context) continue;

            std::shared_ptr<PixelContext> pixel_context = material->GetPixelContext();
            if (!pixel_context) continue;

            // 绑定 Shader 并绘制对应的 RenderPrimitive
            if (rhi->BindVertexContext(vertex_context) && rhi->BindPixelContext(pixel_context))
            {
                g_dolas_engine.m_rhi->DrawRenderPrimitive(component.m_render_primitive_id);
            }
        }
    }

    void RenderEntity::AddComponent(RenderPrimitiveID mesh_id, MaterialID material_id)
    {
        m_components.push_back({ mesh_id, material_id });
    }
}
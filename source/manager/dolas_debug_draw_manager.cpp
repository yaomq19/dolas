#include "manager/dolas_debug_draw_manager.h"
#include "core/dolas_engine.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "render/dolas_shader.h"
#include "manager/dolas_geometry_manager.h"
namespace Dolas
{
    DebugDrawManager::DebugDrawManager()
    {
    }

    DebugDrawManager::~DebugDrawManager()
    {
    }
    
    bool DebugDrawManager::Initialize()
    {
        return true;
    }

    bool DebugDrawManager::Clear()
    {
        return true;
    }

    void DebugDrawManager::Render()
    {
        DolasRHI* rhi = g_dolas_engine.m_rhi;
		DOLAS_RETURN_IF_NULL(rhi);

        Material* debug_draw_material = g_dolas_engine.m_material_manager->GetDebugDrawMaterial();
		DOLAS_RETURN_IF_NULL(debug_draw_material);

        VertexContext* vertex_context = debug_draw_material->GetVertexContext();
        PixelContext* pixel_context = debug_draw_material->GetPixelContext();

        for (const DebugDrawObject& debug_draw_object : m_render_objects)
        {
            Vector4 color_value(
                debug_draw_object.m_color.r,
                debug_draw_object.m_color.g,
                debug_draw_object.m_color.b,
				debug_draw_object.m_color.a);

			pixel_context->SetGlobalVariable("g_DebugDrawColor", color_value);
            if (rhi->BindVertexContext(vertex_context) && rhi->BindPixelContext(pixel_context))
            {
                rhi->DrawRenderPrimitive(debug_draw_object.m_render_primitive_id);
            }
        }
    }

    void DebugDrawManager::Tick(Float delta_time)
    {
		for (auto iter = m_render_objects.begin(); iter != m_render_objects.end();)
		{
            iter->m_life_time -= delta_time;
            if (iter->m_life_time <= 0.0f)
            {
                iter = m_render_objects.erase(iter);
            }
            else
            {
                iter++;
            }
		}
    }

    void DebugDrawManager::AddCylinder(const Vector3& center, const Float radius, const Float height, const Quaternion& rotation, const Color& color, Float life_time)
    {
		auto* geometry_manager = g_dolas_engine.m_geometry_manager;
        DOLAS_RETURN_IF_NULL(geometry_manager);
		DebugDrawObject cylinder;
        cylinder.m_render_primitive_id = geometry_manager->GetGeometryRenderPrimitiveID(BaseGeometryType_CYLINDER);
        cylinder.m_color = color;
        cylinder.m_life_time = life_time;
		m_render_objects.push_back(cylinder);
    }
}// namespace Dolas
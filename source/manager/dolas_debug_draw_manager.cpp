#include "manager/dolas_debug_draw_manager.h"
#include "core/dolas_engine.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_log_system_manager.h"
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
        for (const DebugDrawObject& debug_draw_object : m_render_objects)
        {
            RenderPrimitive* render_primitive = g_dolas_engine.m_render_primitive_manager->GetRenderPrimitiveByID(debug_draw_object.m_render_primitive_id);
            DOLAS_RETURN_IF_NULL(render_primitive);

            debug_draw_object.m_color;
            render_primitive->Draw(g_dolas_engine.m_rhi);
        }
    }

    void DebugDrawManager::Tick(Float delta_time)
    {
		for (auto iter = m_render_objects.begin(); iter != m_render_objects.end();)
		{
            iter->m_life_time -= delta_time;
            if (iter->m_life_time <= 0.0f)
            {
                Bool delete_success = g_dolas_engine.m_render_primitive_manager->DeleteRenderPrimitiveByID(iter->m_render_primitive_id);
                if (!delete_success)
                {
                    LOG_ERROR("Failed to delete render primitive : {0}", iter->m_render_primitive_id);
                }
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
        // todo
    }
}// namespace Dolas
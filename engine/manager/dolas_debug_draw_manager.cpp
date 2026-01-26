#include "manager/dolas_debug_draw_manager.h"
#include "core/dolas_engine.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "render/dolas_shader.h"
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

    void DebugDrawManager::TickPostRender(Float delta_time)
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
		auto* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
        DOLAS_RETURN_IF_NULL(render_primitive_manager);
		DebugDrawObject cylinder;
        cylinder.m_render_primitive_id = render_primitive_manager->GetGeometryRenderPrimitiveID(BaseGeometryType_CYLINDER);
		cylinder.m_pose.m_postion = center;
		cylinder.m_pose.m_rotation = rotation;
		cylinder.m_pose.m_scale = Vector3(radius, height, radius);
        cylinder.m_color = color;
        cylinder.m_life_time = life_time;
		m_render_objects.push_back(cylinder);
    }

    void DebugDrawManager::AddSphere(const Vector3& center, const Float radius, const Color& color, Float life_time)
    {
		auto* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
		DOLAS_RETURN_IF_NULL(render_primitive_manager);
		DebugDrawObject sphere;
        sphere.m_render_primitive_id = render_primitive_manager->GetGeometryRenderPrimitiveID(BaseGeometryType_SPHERE);
		sphere.m_pose.m_postion = center;
		sphere.m_pose.m_rotation = Quaternion::IDENTITY;
		sphere.m_pose.m_scale = Vector3(radius, radius, radius);
        sphere.m_color = color;
        sphere.m_life_time = life_time;
		m_render_objects.push_back(sphere);
    }
}// namespace Dolas
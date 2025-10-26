#include "manager/dolas_geometry_manager.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_primitive_manager.h"
namespace Dolas
{

    GeometryManager::GeometryManager()
    {

    }

    GeometryManager::~GeometryManager()
    {

    }
    
    bool GeometryManager::Initialize()
    {
        RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;

        std::vector<Float> vertices_data;
        std::vector<UInt> indices_data;
        
		StringID sphere_render_primitive_string_id = STRING_ID(sphere_render_primitive);
		Bool success = render_primitive_manager->CreateRenderPrimitive(
            sphere_render_primitive_string_id,
			RenderPrimitiveTopology::_TRIANGLE_LIST,
			InputLayoutType::POS_3,
			vertices_data,
			indices_data);
        if (success)
        {
            m_geometries[BaseGeometryType::_SPHERE] = sphere_render_primitive_string_id;
        }
        return true;
    }

    bool GeometryManager::Clear()
    {
        return true;
    }

    RenderPrimitiveID GeometryManager::GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type)
    {
        if (m_geometries.find(geometry_type) != m_geometries.end())
        {
			return m_geometries[geometry_type];
        }
        else
        {
			return RENDER_PRIMITIVE_ID_EMPTY;
        }
    }

} // namespace Dolas
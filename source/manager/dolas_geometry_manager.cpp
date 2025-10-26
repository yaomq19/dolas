#include "manager/dolas_geometry_manager.h"

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
        /*RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
        m_geometries[BaseGeometryType::_TRIANGLE] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(triangle_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
        m_geometries[BaseGeometryType::_QUAD] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(quad_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
        m_geometries[BaseGeometryType::_CUBE] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(cube_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
        m_geometries[BaseGeometryType::_SPHERE] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(sphere_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
        m_geometries[BaseGeometryType::_CYLINDER] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(cylinder_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
        m_geometries[BaseGeometryType::_CONE] = render_primitive_manager->CreateRenderPrimitive(STRING_ID(cone_render_primitive), DolasRenderPrimitiveType::_TRIANGLE_LIST, DolasInputLayoutType::_POSITION_COLOR, { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });*/
        return true;
    }

    bool GeometryManager::Clear()
    {
        return true;
    }

    RenderPrimitiveID GeometryManager::GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type)
    {
        return 0;
    }

} // namespace Dolas
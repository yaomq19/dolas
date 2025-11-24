#include "manager/dolas_geometry_manager.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_log_system_manager.h"
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
        Bool success = generateSphereGeometry(10, vertices_data, indices_data);
        if (!success)
        {
            LOG_ERROR("GeometryManager::Initialize: Failed to generate sphere geometry");
            return false;
        }
		RenderPrimitiveID sphere_render_primitive_string_id = STRING_ID(sphere_render_primitive);
		success = render_primitive_manager->CreateRenderPrimitive(
            sphere_render_primitive_string_id,
			PrimitiveTopology::PrimitiveTopology_TriangleList,
			InputLayoutType::InputLayoutType_POS_3,
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

    Bool GeometryManager::generateSphereGeometry(UInt segments, std::vector<Float>& vertices, std::vector<UInt>& indices)
    {
        // 验证 segments 参数范围
        if (segments < 1 || segments > 100)
        {
            LOG_ERROR("GeometryManager::generateSphereGeometry: Invalid segments value: {}, valid range is [1, 100]", segments);
            return false;
        }

        vertices.clear();
        indices.clear();

        const Float PI = 3.14159265358979323846f;
        const Float radius = 1.0f;

        // 计算分段数
        // latitudeSegments: 纬度方向的分段数（从北极到南极）
        // longitudeSegments: 经度方向的分段数（环绕球体）
        UInt latitudeSegments = segments;
        UInt longitudeSegments = segments * 2; // 经度分段数通常是纬度的2倍，使三角形更接近正方形

        // 生成顶点
        // 顶点总数 = (latitudeSegments + 1) * (longitudeSegments + 1)
        for (UInt lat = 0; lat <= latitudeSegments; ++lat)
        {
            // phi: 纬度角，范围 [0, PI]，从北极 (0) 到南极 (PI)
            Float phi = static_cast<Float>(lat) / static_cast<Float>(latitudeSegments) * PI;
            Float sinPhi = sin(phi);
            Float cosPhi = cos(phi);

            for (UInt lon = 0; lon <= longitudeSegments; ++lon)
            {
                // theta: 经度角，范围 [0, 2*PI]
                Float theta = static_cast<Float>(lon) / static_cast<Float>(longitudeSegments) * 2.0f * PI;
                Float sinTheta = sin(theta);
                Float cosTheta = cos(theta);

                // 球坐标转笛卡尔坐标
                // x = r * sin(phi) * cos(theta)
                // y = r * cos(phi)
                // z = r * sin(phi) * sin(theta)
                Float x = radius * sinPhi * cosTheta;
                Float y = radius * cosPhi;
                Float z = radius * sinPhi * sinTheta;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        // 生成索引（三角形列表）
        for (UInt lat = 0; lat < latitudeSegments; ++lat)
        {
            for (UInt lon = 0; lon < longitudeSegments; ++lon)
            {
                // 当前四边形的四个顶点索引
                UInt first = lat * (longitudeSegments + 1) + lon;
                UInt second = first + longitudeSegments + 1;

                // 第一个三角形 (逆时针方向)
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                // 第二个三角形 (逆时针方向)
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        LOG_INFO("GeometryManager::generateSphereGeometry: Generated sphere with {} vertices and {} triangles", 
                 vertices.size() / 3, indices.size() / 3);

        return true;
    }

} // namespace Dolas
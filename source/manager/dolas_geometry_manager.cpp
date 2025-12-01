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
        Bool initialize_result = true;
        initialize_result &= InitializeSphereGeometry();
        initialize_result &= InitializeQuadGeometry();
		initialize_result &= InitializeCylinderGeometry();

        return true;
    }

    bool GeometryManager::Clear()
    {
        return true;
    }

    RenderPrimitiveID GeometryManager::GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type)
    {
		RenderPrimitiveID render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
        auto iter = m_geometries.find(geometry_type);
        if (iter != m_geometries.end())
        {
            render_primitive_id = iter->second;
        }
		return render_primitive_id;
    }

    Bool GeometryManager::InitializeSphereGeometry()
    {
        RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
        DOLAS_RETURN_FALSE_IF_NULL(render_primitive_manager);

        std::vector<std::vector<Float>> vertices_data;
        std::vector<UInt> indices_data;
        if (!GenerateSphereRawData(10, vertices_data, indices_data))
        {
            LOG_ERROR("Failed to generate sphere Raw Data");
            return false;
        }
        RenderPrimitiveID sphere_render_primitive_string_id = STRING_ID(sphere_render_primitive);
        Bool success = render_primitive_manager->CreateRenderPrimitive(
            sphere_render_primitive_string_id,
            PrimitiveTopology::PrimitiveTopology_TriangleList,
            InputLayoutType::InputLayoutType_POS_3,
            vertices_data,
            indices_data);
        if (!success)
        {
            LOG_ERROR("Failed to CreateRenderPrimitive");
            return false;
        }

        m_geometries[BaseGeometryType_SPHERE] = sphere_render_primitive_string_id;
    }

    Bool GeometryManager::InitializeQuadGeometry()
    {
        RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
        DOLAS_RETURN_FALSE_IF_NULL(render_primitive_manager);

        std::vector<std::vector<Float>> vertices_data;
        std::vector<UInt> indices_data;
        if (!GenerateQuadRawData(vertices_data, indices_data))
        {
            LOG_ERROR("Failed to generate quad Raw Data");
            return false;
        }
        RenderPrimitiveID quad_render_primitive_id = STRING_ID(quad_render_primitive);
        Bool success = render_primitive_manager->CreateRenderPrimitive(
            quad_render_primitive_id,
            PrimitiveTopology::PrimitiveTopology_TriangleList,
            InputLayoutType::InputLayoutType_POS_3_UV_2,
            vertices_data,
            indices_data);
        if (!success)
        {
            LOG_ERROR("Failed to CreateRenderPrimitive");
            return false;
        }

        m_geometries[BaseGeometryType_QUAD] = quad_render_primitive_id;
    }

    Bool GeometryManager::InitializeCylinderGeometry()
    {
        RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
        DOLAS_RETURN_FALSE_IF_NULL(render_primitive_manager);

        std::vector<std::vector<Float>> vertices_data;
        std::vector<UInt> indices_data;
        if (!GenerateCylinderRawData(vertices_data, indices_data))
        {
            LOG_ERROR("Failed to generate quad Raw Data");
            return false;
        }
        RenderPrimitiveID cylinder_render_primitive_id = STRING_ID(cylinder_render_primitive);
        Bool success = render_primitive_manager->CreateRenderPrimitive(
            cylinder_render_primitive_id,
            PrimitiveTopology::PrimitiveTopology_TriangleList,
            InputLayoutType::InputLayoutType_POS_3,
            vertices_data,
            indices_data);
        if (!success)
        {
            LOG_ERROR("Failed to CreateRenderPrimitive");
            return false;
        }

        m_geometries[BaseGeometryType_CYLINDER] = cylinder_render_primitive_id;
    }

    Bool GeometryManager::GenerateSphereRawData(UInt segments, std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
    {
        // 验证 segments 参数范围
        if (segments < 1 || segments > 100)
        {
            LOG_ERROR("Invalid segments value: {}, valid range is [1, 100]", segments);
            return false;
        }

        vertices_data.clear();
        indices.clear();

		vertices_data.resize(1); // 只使用一种顶点格式，这里使用第一个元素存储位置数据
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

                vertices_data[0].push_back(x);
                vertices_data[0].push_back(y);
                vertices_data[0].push_back(z);
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

        return true;
    }

    Bool GeometryManager::GenerateQuadRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
    {
        vertices_data.clear();
        indices.clear();

        // 对应 InputLayoutType_POS_3_UV_2：
        // stream 0: position (x, y, z)
        // stream 1: uv       (u, v)
        vertices_data.resize(2);

        // 在 XY 平面上的单位四边形，中心在原点，面朝 +Z 方向
        // 顶点顺序：
        // 0: (-1, -1, 0)  uv (0, 1)
        // 1: (-1,  1, 0)  uv (0, 0)
        // 2: ( 1, -1, 0)  uv (1, 1)
        // 3: ( 1,  1, 0)  uv (1, 0)

        // positions (stream 0)
        vertices_data[0].push_back(-1.0f); // v0
        vertices_data[0].push_back(-1.0f);
        vertices_data[0].push_back(0.5f);

        vertices_data[0].push_back(-1.0f); // v1
        vertices_data[0].push_back( 1.0f);
        vertices_data[0].push_back(0.5f);

        vertices_data[0].push_back( 1.0f); // v2
        vertices_data[0].push_back(-1.0f);
        vertices_data[0].push_back(0.5f);

        vertices_data[0].push_back( 1.0f); // v3
        vertices_data[0].push_back( 1.0f);
        vertices_data[0].push_back(0.5f);

        // uvs (stream 1)
        vertices_data[1].push_back(0.0f); // v0
        vertices_data[1].push_back(1.0f);

        vertices_data[1].push_back(0.0f); // v1
        vertices_data[1].push_back(0.0f);

        vertices_data[1].push_back(1.0f); // v2
        vertices_data[1].push_back(1.0f);

        vertices_data[1].push_back(1.0f); // v3
        vertices_data[1].push_back(0.0f);

        // 索引（TriangleList，逆时针，面朝 +Z）
        // 三角形 1: v0, v2, v1
        // 三角形 2: v2, v3, v1
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(1);

        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(1);

        return true;
    }

    Bool GeometryManager::GenerateCylinderRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
    {
        vertices_data.clear();
        indices.clear();

        // 使用单一顶点流：positions（x, y, z），对应 InputLayoutType_POS_3
        vertices_data.resize(1);
        std::vector<Float>& positions = vertices_data[0];

        const Float radius      = 1.0f;
        const Float half_height = 1.0f;
        const UInt  segments    = 32;   // 圆周分段数
        const Float PI          = MathUtil::PI;

        // ---------- 1. 侧面顶点（上环 + 下环） ----------
        // 顶环 [0 .. segments]
        for (UInt i = 0; i <= segments; ++i)
        {
            Float theta   = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
            Float cosTheta = cos(theta);
            Float sinTheta = sin(theta);

            Float x = radius * cosTheta;
            Float z = radius * sinTheta;
            Float y = half_height;

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }

        // 底环 [segments+1 .. 2*segments+1]
        const UInt bottom_ring_start = segments + 1;
        for (UInt i = 0; i <= segments; ++i)
        {
            Float theta   = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
            Float cosTheta = cos(theta);
            Float sinTheta = sin(theta);

            Float x = radius * cosTheta;
            Float z = radius * sinTheta;
            Float y = -half_height;

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }

        // ---------- 2. 侧面索引（三角形列表） ----------
        for (UInt i = 0; i < segments; ++i)
        {
            UInt top_curr    = i;
            UInt top_next    = i + 1;
            UInt bottom_curr = bottom_ring_start + i;
            UInt bottom_next = bottom_ring_start + i + 1;

            // 第一个三角形（逆时针，从外侧看）
            indices.push_back(top_curr);
            indices.push_back(bottom_curr);
            indices.push_back(top_next);

            // 第二个三角形
            indices.push_back(top_next);
            indices.push_back(bottom_curr);
            indices.push_back(bottom_next);
        }

        // ---------- 3. 端盖顶点：顶端 + 底端 ----------
        // 顶盖中心
        const UInt top_center_index = static_cast<UInt>(positions.size() / 3);
        positions.push_back(0.0f);
        positions.push_back(half_height);
        positions.push_back(0.0f);

        // 顶盖环（重复一圈，以便索引逻辑和侧面一样简单）
        const UInt top_cap_ring_start = static_cast<UInt>(positions.size() / 3);
        for (UInt i = 0; i <= segments; ++i)
        {
            Float theta   = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
            Float cosTheta = cos(theta);
            Float sinTheta = sin(theta);

            Float x = radius * cosTheta;
            Float z = radius * sinTheta;
            Float y = half_height;

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }

        // 底盖中心
        const UInt bottom_center_index = static_cast<UInt>(positions.size() / 3);
        positions.push_back(0.0f);
        positions.push_back(-half_height);
        positions.push_back(0.0f);

        // 底盖环
        const UInt bottom_cap_ring_start = static_cast<UInt>(positions.size() / 3);
        for (UInt i = 0; i <= segments; ++i)
        {
            Float theta   = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
            Float cosTheta = cos(theta);
            Float sinTheta = sin(theta);

            Float x = radius * cosTheta;
            Float z = radius * sinTheta;
            Float y = -half_height;

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }

        // ---------- 4. 端盖索引（顶盖朝 +Y，底盖朝 -Y） ----------
        // 顶盖：从上往下看，逆时针
        for (UInt i = 0; i < segments; ++i)
        {
            UInt v0 = top_center_index;
            UInt v1 = top_cap_ring_start + i;
            UInt v2 = top_cap_ring_start + i + 1;

            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
        }

        // 底盖：从上往下看，需要顺时针，等价于交换 v1/v2
        for (UInt i = 0; i < segments; ++i)
        {
            UInt v0 = bottom_center_index;
            UInt v1 = bottom_cap_ring_start + i + 1;
            UInt v2 = bottom_cap_ring_start + i;

            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
        }

        return true;
    }
} // namespace Dolas
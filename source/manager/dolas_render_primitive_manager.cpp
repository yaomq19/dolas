#include "manager/dolas_render_primitive_manager.h"

#include <iostream>

#include "common/dolas_hash.h"
#include "core/dolas_engine.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_render_primitive.h"
#include "base/dolas_paths.h"
#include "nlohmann/json.hpp"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "core/dolas_rhi_common.h"
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
    using json = nlohmann::json;
    RenderPrimitiveManager::RenderPrimitiveManager()
    {
    }

    RenderPrimitiveManager::~RenderPrimitiveManager()
    {
    }

    Bool RenderPrimitiveManager::Initialize()
    {
		Bool initialize_result = true;
		initialize_result &= InitializeSphereGeometry();
		initialize_result &= InitializeQuadGeometry();
		initialize_result &= InitializeCylinderGeometry();
		initialize_result &= InitializeCubeGeometry();

        return initialize_result;
    }

    Bool RenderPrimitiveManager::Clear()
    {
		m_base_geometries.clear();
        m_render_primitives.clear();
        return true;
    }

	RenderPrimitiveID RenderPrimitiveManager::GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type)
	{
		RenderPrimitiveID render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
		auto iter = m_base_geometries.find(geometry_type);
		if (iter != m_base_geometries.end())
		{
			render_primitive_id = iter->second;
		}
		return render_primitive_id;
	}

    RenderPrimitiveID RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile(const std::string& mesh_file_name)
    {
        std::string mesh_file_path = PathUtils::GetMeshDir() + mesh_file_name;

        json json_data;
        Bool ret = g_dolas_engine.m_asset_manager->LoadJsonFile(mesh_file_path, json_data);
        DOLAS_RETURN_FALSE_IF_FALSE(ret);

        RenderPrimitiveID primitive_id = HashConverter::StringHash(mesh_file_path);

        // 验证顶点数量
        if (!json_data.contains("vertex_count") || !json_data.contains("vertex_list"))
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: vertex_count or vertex_list not found in {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }

        int vertex_count = json_data["vertex_count"];
        const auto& vertex_list = json_data["vertex_list"];

        if (vertex_count != vertex_list.size())
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: vertex_count mismatch in {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }

        // 解析 IA
        if (!json_data.contains("input_layout"))
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: input_layout is Empty {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }
        const auto& input_layout = json_data["input_layout"];
        Bool has_position = false;
        Bool has_uv = false;
        Bool has_normal = false;
        int pos_element_count = 0;
        int uv_element_count = 0;
        int normal_element_count = 0;

        if (input_layout.contains("position"))
        {
            has_position = true;
            pos_element_count = input_layout["position"];
        }

        if (input_layout.contains("uv"))
        {
            has_uv = true;
            uv_element_count = input_layout["uv"];
        }

        if (input_layout.contains("normal"))
        {
            has_normal = true;
            normal_element_count = input_layout["normal"];
        }

        std::vector<std::vector<Float>> final_vertices;
        InputLayoutType input_layout_type = InputLayoutType::InputLayoutType_POS_3;

        if (has_position && has_uv && has_normal)
        {
            final_vertices.resize(3);
            if (pos_element_count == 3 && uv_element_count == 2 && normal_element_count == 3)
            {
                input_layout_type = InputLayoutType::InputLayoutType_POS_3_UV_2_NORM_3;
                for (const auto& vertex : vertex_list)
                {
                    const auto& pos = vertex["position"];
                    const auto& uv = vertex["uv"];
                    const auto& normal = vertex["normal"];

                    final_vertices[0].push_back(pos[0]);
                    final_vertices[0].push_back(pos[1]);
                    final_vertices[0].push_back(pos[2]);

                    final_vertices[1].push_back(uv[0]);
                    final_vertices[1].push_back(uv[1]);

                    final_vertices[2].push_back(normal[0]);
                    final_vertices[2].push_back(normal[1]);
                    final_vertices[2].push_back(normal[2]);
                }
            }
            else
            {
                LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
                return RENDER_PRIMITIVE_ID_EMPTY;
            }
        }
        else if (has_position && has_uv)
        {
            final_vertices.resize(2);
            if (pos_element_count == 3 && uv_element_count == 2)
            {
                input_layout_type = InputLayoutType::InputLayoutType_POS_3_UV_2;
                for (const auto& vertex : vertex_list)
                {
                    const auto& pos = vertex["position"];
                    const auto& uv = vertex["uv"];

                    final_vertices[0].push_back(pos[0]);
                    final_vertices[0].push_back(pos[1]);
                    final_vertices[0].push_back(pos[2]);

                    final_vertices[1].push_back(uv[0]);
                    final_vertices[1].push_back(uv[1]);
                }
            }
            else
            {
                LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
                return RENDER_PRIMITIVE_ID_EMPTY;
            }
        }
		else if (has_position && has_normal)
		{
			final_vertices.resize(2);
			if (pos_element_count == 3 && normal_element_count == 3)
			{
				input_layout_type = InputLayoutType::InputLayoutType_POS_3_NORM_3;
				for (const auto& vertex : vertex_list)
				{
					const auto& pos = vertex["position"];
					const auto& normal = vertex["normal"];

					final_vertices[0].push_back(pos[0]);
					final_vertices[0].push_back(pos[1]);
					final_vertices[0].push_back(pos[2]);

					final_vertices[1].push_back(normal[0]);
					final_vertices[1].push_back(normal[1]);
					final_vertices[1].push_back(normal[2]);
				}
			}
			else
			{
				LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
				return RENDER_PRIMITIVE_ID_EMPTY;
			}
		}
		else
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: input_layout missing required elements in {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }

        // 解析索引数据
        std::vector<UInt> indices;
        if (json_data.contains("index_count") && json_data.contains("index_list"))
        {
            int index_count = json_data["index_count"];
            const auto& index_list = json_data["index_list"];

            if (index_count != index_list.size())
            {
                LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: index_count mismatch in {0}", mesh_file_path);
                return RENDER_PRIMITIVE_ID_EMPTY;
            }

            indices.reserve(index_count);
            for (const auto& index : index_list)
            {
                indices.push_back(index);
            }
        }

        // 解析 Primitive
        if (!json_data.contains("primitive"))
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: primitive is Empty {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }
        const std::string& primitive_str = json_data["primitive"];
        PrimitiveTopology primitive_topology = PrimitiveTopology::PrimitiveTopology_TriangleList;
        if (primitive_str == "TriangleList")
        {
            primitive_topology = PrimitiveTopology::PrimitiveTopology_TriangleList;
        }
        else
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported primitive type in {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }

        Bool success = CreateRenderPrimitive(
            primitive_id,
            primitive_topology,
            input_layout_type,
            final_vertices,
            indices);
        if (!success)
        {
            LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: failed to create render primitive for {0}", mesh_file_path);
            return RENDER_PRIMITIVE_ID_EMPTY;
        }

        return primitive_id;
    }

	Bool RenderPrimitiveManager::InitializeSphereGeometry()
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

		m_base_geometries[BaseGeometryType_SPHERE] = sphere_render_primitive_string_id;
	}

	Bool RenderPrimitiveManager::InitializeQuadGeometry()
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

		m_base_geometries[BaseGeometryType_QUAD] = quad_render_primitive_id;
	}

	Bool RenderPrimitiveManager::InitializeCylinderGeometry()
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

		m_base_geometries[BaseGeometryType_CYLINDER] = cylinder_render_primitive_id;
	}

	Bool RenderPrimitiveManager::InitializeCubeGeometry()
	{
		RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_primitive_manager);

		std::vector<std::vector<Float>> vertices_data;
		std::vector<UInt> indices_data;

		if (!GenerateCubeRawData(vertices_data, indices_data))
		{
			LOG_ERROR("Failed to generate cube Raw Data");
			return false;
		}

		RenderPrimitiveID cube_render_primitive_id = STRING_ID(cube_render_primitive);
		Bool success = render_primitive_manager->CreateRenderPrimitive(
			cube_render_primitive_id,
			PrimitiveTopology::PrimitiveTopology_TriangleList,
			InputLayoutType::InputLayoutType_POS_3_NORM_3,
			vertices_data,
			indices_data);

		if (!success)
		{
			LOG_ERROR("Failed to CreateRenderPrimitive");
			return false;
		}

		m_base_geometries[BaseGeometryType_CUBE] = cube_render_primitive_id;
	}

	Bool RenderPrimitiveManager::GenerateSphereRawData(UInt segments, std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
	{
		// ��֤ segments ������Χ
		if (segments < 1 || segments > 100)
		{
			LOG_ERROR("Invalid segments value: {}, valid range is [1, 100]", segments);
			return false;
		}

		vertices_data.clear();
		indices.clear();

		vertices_data.resize(1); // ֻʹ��һ�ֶ����ʽ������ʹ�õ�һ��Ԫ�ش洢λ������
		const Float PI = 3.14159265358979323846f;
		const Float radius = 1.0f;

		// ����ֶ���
		// latitudeSegments: γ�ȷ���ķֶ������ӱ������ϼ���
		// longitudeSegments: ���ȷ���ķֶ������������壩
		UInt latitudeSegments = segments;
		UInt longitudeSegments = segments * 2; // ���ȷֶ���ͨ����γ�ȵ�2����ʹ�����θ��ӽ�������

		// ���ɶ���
		// �������� = (latitudeSegments + 1) * (longitudeSegments + 1)
		for (UInt lat = 0; lat <= latitudeSegments; ++lat)
		{
			// phi: γ�Ƚǣ���Χ [0, PI]���ӱ��� (0) ���ϼ� (PI)
			Float phi = static_cast<Float>(lat) / static_cast<Float>(latitudeSegments) * PI;
			Float sinPhi = sin(phi);
			Float cosPhi = cos(phi);

			for (UInt lon = 0; lon <= longitudeSegments; ++lon)
			{
				// theta: ���Ƚǣ���Χ [0, 2*PI]
				Float theta = static_cast<Float>(lon) / static_cast<Float>(longitudeSegments) * 2.0f * PI;
				Float sinTheta = sin(theta);
				Float cosTheta = cos(theta);

				// ������ת�ѿ�������
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

		// �����������������б���
		for (UInt lat = 0; lat < latitudeSegments; ++lat)
		{
			for (UInt lon = 0; lon < longitudeSegments; ++lon)
			{
				// ��ǰ�ı��ε��ĸ���������
				UInt first = lat * (longitudeSegments + 1) + lon;
				UInt second = first + longitudeSegments + 1;

				// ��һ�������� (��ʱ�뷽��)
				indices.push_back(first);
				indices.push_back(second);
				indices.push_back(first + 1);

				// �ڶ��������� (��ʱ�뷽��)
				indices.push_back(second);
				indices.push_back(second + 1);
				indices.push_back(first + 1);
			}
		}

		return true;
	}

	Bool RenderPrimitiveManager::GenerateQuadRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
	{
		vertices_data.clear();
		indices.clear();

		// ��Ӧ InputLayoutType_POS_3_UV_2��
		// stream 0: position (x, y, z)
		// stream 1: uv       (u, v)
		vertices_data.resize(2);

		// �� XY ƽ���ϵĵ�λ�ı��Σ�������ԭ�㣬�泯 +Z ����
		// ����˳��
		// 0: (-1, -1, 0)  uv (0, 1)
		// 1: (-1,  1, 0)  uv (0, 0)
		// 2: ( 1, -1, 0)  uv (1, 1)
		// 3: ( 1,  1, 0)  uv (1, 0)

		// positions (stream 0)
		vertices_data[0].push_back(-1.0f); // v0
		vertices_data[0].push_back(-1.0f);
		vertices_data[0].push_back(0.5f);

		vertices_data[0].push_back(-1.0f); // v1
		vertices_data[0].push_back(1.0f);
		vertices_data[0].push_back(0.5f);

		vertices_data[0].push_back(1.0f); // v2
		vertices_data[0].push_back(-1.0f);
		vertices_data[0].push_back(0.5f);

		vertices_data[0].push_back(1.0f); // v3
		vertices_data[0].push_back(1.0f);
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

		// ������TriangleList����ʱ�룬�泯 +Z��
		// ������ 1: v0, v2, v1
		// ������ 2: v2, v3, v1
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(1);

		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(1);

		return true;
	}

	Bool RenderPrimitiveManager::GenerateCylinderRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
	{
		vertices_data.clear();
		indices.clear();

		// ʹ�õ�һ��������positions��x, y, z������Ӧ InputLayoutType_POS_3
		vertices_data.resize(1);
		std::vector<Float>& positions = vertices_data[0];

		const Float radius = 1.0f;
		const Float half_height = 0.5f;
		const UInt  segments = 32;   // Բ�ֶܷ���
		const Float PI = MathUtil::PI;

		// ---------- 1. ���涥�㣨�ϻ� + �»��� ----------
		// ���� [0 .. segments]
		for (UInt i = 0; i <= segments; ++i)
		{
			Float theta = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float cosTheta = cos(theta);
			Float sinTheta = sin(theta);

			Float x = radius * cosTheta;
			Float z = radius * sinTheta;
			Float y = half_height;

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
		}

		// �׻� [segments+1 .. 2*segments+1]
		const UInt bottom_ring_start = segments + 1;
		for (UInt i = 0; i <= segments; ++i)
		{
			Float theta = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float cosTheta = cos(theta);
			Float sinTheta = sin(theta);

			Float x = radius * cosTheta;
			Float z = radius * sinTheta;
			Float y = -half_height;

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
		}

		// ---------- 2. �����������������б��� ----------
		for (UInt i = 0; i < segments; ++i)
		{
			UInt top_curr = i;
			UInt top_next = i + 1;
			UInt bottom_curr = bottom_ring_start + i;
			UInt bottom_next = bottom_ring_start + i + 1;

			// ��һ�������Σ���ʱ�룬����࿴��
			indices.push_back(top_curr);
			indices.push_back(bottom_curr);
			indices.push_back(top_next);

			// �ڶ���������
			indices.push_back(top_next);
			indices.push_back(bottom_curr);
			indices.push_back(bottom_next);
		}

		// ---------- 3. �˸Ƕ��㣺���� + �׶� ----------
		// ��������
		const UInt top_center_index = static_cast<UInt>(positions.size() / 3);
		positions.push_back(0.0f);
		positions.push_back(half_height);
		positions.push_back(0.0f);

		// ���ǻ����ظ�һȦ���Ա������߼��Ͳ���һ���򵥣�
		const UInt top_cap_ring_start = static_cast<UInt>(positions.size() / 3);
		for (UInt i = 0; i <= segments; ++i)
		{
			Float theta = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float cosTheta = cos(theta);
			Float sinTheta = sin(theta);

			Float x = radius * cosTheta;
			Float z = radius * sinTheta;
			Float y = half_height;

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
		}

		// �׸�����
		const UInt bottom_center_index = static_cast<UInt>(positions.size() / 3);
		positions.push_back(0.0f);
		positions.push_back(-half_height);
		positions.push_back(0.0f);

		// �׸ǻ�
		const UInt bottom_cap_ring_start = static_cast<UInt>(positions.size() / 3);
		for (UInt i = 0; i <= segments; ++i)
		{
			Float theta = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float cosTheta = cos(theta);
			Float sinTheta = sin(theta);

			Float x = radius * cosTheta;
			Float z = radius * sinTheta;
			Float y = -half_height;

			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
		}

		// ---------- 4. �˸����������ǳ� +Y���׸ǳ� -Y�� ----------
		// ���ǣ��������¿�����ʱ��
		for (UInt i = 0; i < segments; ++i)
		{
			UInt v0 = top_center_index;
			UInt v1 = top_cap_ring_start + i;
			UInt v2 = top_cap_ring_start + i + 1;

			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
		}

		// �׸ǣ��������¿�����Ҫ˳ʱ�룬�ȼ��ڽ��� v1/v2
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

	Bool RenderPrimitiveManager::GenerateCubeRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
	{
		vertices_data.clear();
		indices.clear();

		// 使用双顶点流：
		//   stream 0: positions（x, y, z）
		//   stream 1: normals  （x, y, z）
		// 对应 InputLayoutType_POS_3_NORM_3
		vertices_data.resize(2);
		std::vector<Float>& positions = vertices_data[0];

		const Float half_extent = 0.5f;

		// 8 个顶点，立方体中心在原点，边长为 1（范围 [-0.5, 0.5]）
		// v0: (-x, -y, -z)
		positions.push_back(-half_extent);
		positions.push_back(-half_extent);
		positions.push_back(-half_extent);

		// v1: (-x, +y, -z)
		positions.push_back(-half_extent);
		positions.push_back(half_extent);
		positions.push_back(-half_extent);

		// v2: (+x, +y, -z)
		positions.push_back(half_extent);
		positions.push_back(half_extent);
		positions.push_back(-half_extent);

		// v3: (+x, -y, -z)
		positions.push_back(half_extent);
		positions.push_back(-half_extent);
		positions.push_back(-half_extent);

		// v4: (-x, -y, +z)
		positions.push_back(-half_extent);
		positions.push_back(-half_extent);
		positions.push_back(half_extent);

		// v5: (-x, +y, +z)
		positions.push_back(-half_extent);
		positions.push_back(half_extent);
		positions.push_back(half_extent);

		// v6: (+x, +y, +z)
		positions.push_back(half_extent);
		positions.push_back(half_extent);
		positions.push_back(half_extent);

		// v7: (+x, -y, +z)
		positions.push_back(half_extent);
		positions.push_back(-half_extent);
		positions.push_back(half_extent);

		// 6 个面，每个面 2 个三角形，共 12 个三角形（36 个索引）
		
		// 前面 (+Z)：v4, v5, v6, v7
		indices.push_back(4); indices.push_back(5); indices.push_back(6);
		indices.push_back(4); indices.push_back(6); indices.push_back(7);

		// 后面 (-Z)：v0, v1, v2, v3
		indices.push_back(3); indices.push_back(2); indices.push_back(1);
		indices.push_back(3); indices.push_back(1); indices.push_back(0);

		// 左面 (-X)：v0, v1, v5, v4
		indices.push_back(0); indices.push_back(1); indices.push_back(5);
		indices.push_back(0); indices.push_back(5); indices.push_back(4);

		// 右面 (+X)：v3, v2, v6, v7
		indices.push_back(7); indices.push_back(6); indices.push_back(2);
		indices.push_back(7); indices.push_back(2); indices.push_back(3);

		// 顶面 (+Y)：v1, v2, v6, v5
		indices.push_back(1); indices.push_back(2); indices.push_back(6);
		indices.push_back(1); indices.push_back(6); indices.push_back(5);

		// 底面 (-Y)：v0, v4, v7, v3
		indices.push_back(0); indices.push_back(4); indices.push_back(7);
		indices.push_back(0); indices.push_back(7); indices.push_back(3);

		// 计算每个顶点的法线（按面法线加权平均再归一化）
		const size_t vertex_count = positions.size() / 3;
		std::vector<Float>& normals = vertices_data[1];
		normals.assign(vertex_count * 3, 0.0f);

		for (size_t i = 0; i + 2 < indices.size(); i += 3)
		{
			UInt i0 = indices[i + 0];
			UInt i1 = indices[i + 1];
			UInt i2 = indices[i + 2];

			Float p0x = positions[i0 * 3 + 0];
			Float p0y = positions[i0 * 3 + 1];
			Float p0z = positions[i0 * 3 + 2];

			Float p1x = positions[i1 * 3 + 0];
			Float p1y = positions[i1 * 3 + 1];
			Float p1z = positions[i1 * 3 + 2];

			Float p2x = positions[i2 * 3 + 0];
			Float p2y = positions[i2 * 3 + 1];
			Float p2z = positions[i2 * 3 + 2];

			// 计算三角形面法线
			Float ux = p1x - p0x;
			Float uy = p1y - p0y;
			Float uz = p1z - p0z;

			Float vx = p2x - p0x;
			Float vy = p2y - p0y;
			Float vz = p2z - p0z;

			Float nx = uy * vz - uz * vy;
			Float ny = uz * vx - ux * vz;
			Float nz = ux * vy - uy * vx;

			// 累加到三个顶点的法线向量上
			normals[i0 * 3 + 0] += nx;
			normals[i0 * 3 + 1] += ny;
			normals[i0 * 3 + 2] += nz;

			normals[i1 * 3 + 0] += nx;
			normals[i1 * 3 + 1] += ny;
			normals[i1 * 3 + 2] += nz;

			normals[i2 * 3 + 0] += nx;
			normals[i2 * 3 + 1] += ny;
			normals[i2 * 3 + 2] += nz;
		}

		// 对每个顶点的法线做归一化
		for (size_t v = 0; v < vertex_count; ++v)
		{
			Float nx = normals[v * 3 + 0];
			Float ny = normals[v * 3 + 1];
			Float nz = normals[v * 3 + 2];

			Float len_sq = nx * nx + ny * ny + nz * nz;
			if (len_sq > 0.0f)
			{
				Float inv_len = 1.0f / std::sqrt(len_sq);
				normals[v * 3 + 0] = nx * inv_len;
				normals[v * 3 + 1] = ny * inv_len;
				normals[v * 3 + 2] = nz * inv_len;
			}
			else
			{
				// 退化情况下给一个默认法线（例如指向 +Y）
				normals[v * 3 + 0] = 0.0f;
				normals[v * 3 + 1] = 1.0f;
				normals[v * 3 + 2] = 0.0f;
			}
		}

		return true;
	}

    RenderPrimitive* RenderPrimitiveManager::BuildFromRawData(
		const PrimitiveTopology& render_primitive_type,
		const InputLayoutType& input_layout_type,
		const std::vector<std::vector<Float>>& vertices,
		const std::vector<UInt>& indices)
    {
        // vertex buffers
        std::vector<BufferID> vertex_buffer_ids;
        std::vector<UInt> vertex_strides; // unit: byte
        std::vector<UInt> vertex_offsets; // unit: byte

        UInt vertex_count = 0;
        for (UInt stream_index = 0; stream_index < vertices.size(); stream_index++)
        {
            UInt vertex_stride = 0; // unit: float
            switch (input_layout_type)
            {
            case InputLayoutType::InputLayoutType_POS_3:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3");
                    return nullptr;
                }
#endif
                break;
            case InputLayoutType::InputLayoutType_POS_3_UV_2:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
                else if (stream_index == 1)
                {
                    vertex_stride = 2;
                }
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3_UV_2");
                    return nullptr;
                }
#endif
                break;
            case InputLayoutType::InputLayoutType_POS_3_UV_2_NORM_3:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
                else if (stream_index == 1)
                {
                    vertex_stride = 2;
                }
                else if (stream_index == 2)
                {
                    vertex_stride = 3;
				}
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3_UV_2_NORM_3");
                    return nullptr;
                }
#endif
                break;
			case InputLayoutType::InputLayoutType_POS_3_NORM_3:
				if (stream_index == 0)
				{
					vertex_stride = 3;
				}
				else if (stream_index == 1)
				{
					vertex_stride = 3;
				}
#ifdef DOLAS_DEBUG
				else
				{
					LOG_ERROR("Vertex stride mismatch: POS_3_UV_2_NORM_3");
					return nullptr;
				}
#endif
				break;
            default:
			    LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Unsupported input layout type");
			    return nullptr;
        }

		    vertex_count = vertices[stream_index].size() / vertex_stride;

            BufferID vertex_buffer_id = g_dolas_engine.m_buffer_manager->CreateVertexBuffer(vertices[stream_index]);
		    if (vertex_buffer_id == BUFFER_ID_EMPTY)
		    {
			    LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create vertex buffer");
			    return nullptr;
		    }
            vertex_buffer_ids.push_back(vertex_buffer_id);
            vertex_strides.push_back(vertex_stride * sizeof(Float)); // convert to byte
			vertex_offsets.push_back(0); // assume offset is 0
        }
        
        // index buffer
        UInt index_count = indices.size();
        BufferID index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer(indices.size() * sizeof(UInt), indices.data());
		if (index_buffer_id == BUFFER_ID_EMPTY)
		{
			LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create index buffer");
			return nullptr;

		}
        RenderPrimitive* render_primitive = DOLAS_NEW(RenderPrimitive);

		render_primitive->m_topology = render_primitive_type;
		render_primitive->m_input_layout_type = input_layout_type;
		render_primitive->m_vertex_strides = vertex_strides;
		render_primitive->m_vertex_offsets = vertex_offsets;
		render_primitive->m_vertex_count = vertex_count;
		render_primitive->m_index_count = index_count;
		render_primitive->m_vertex_buffer_ids = vertex_buffer_ids;
		render_primitive->m_index_buffer_id = index_buffer_id;

        return render_primitive;
    }

    Bool RenderPrimitiveManager::CreateRenderPrimitive(
        RenderPrimitiveID id,
        const PrimitiveTopology& render_primitive_type,
        const InputLayoutType& input_layout_type,
        const std::vector<std::vector<Float>>& vertices,
		const std::vector<UInt>& indices)
    {
		RenderPrimitive* render_primitive = BuildFromRawData(render_primitive_type, input_layout_type, vertices, indices);

        if (render_primitive)
        {
			m_render_primitives[id] = render_primitive;
            return true;
        }
        else
        {
			LOG_ERROR("Failed to build render primitive from raw data");
            return false;
        }
    }

    RenderPrimitive* RenderPrimitiveManager::GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id) const
    {
        if (m_render_primitives.find(render_primitive_id) != m_render_primitives.end())
        {
            return m_render_primitives.at(render_primitive_id);
        }
        return nullptr;
    }
}



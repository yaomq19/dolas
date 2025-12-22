#include "manager/dolas_render_primitive_manager.h"
#include <limits>

#include <iostream>
#include <cmath>
#include <tuple>

#include "common/dolas_hash.h"
#include "core/dolas_engine.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_render_primitive.h"
#include "base/dolas_paths.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "core/dolas_rhi_common.h"
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
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
  //      std::string mesh_file_path = PathUtils::GetMeshDir() + mesh_file_name;

  //      json json_data;
  //      Bool ret = g_dolas_engine.m_asset_manager->LoadJsonFile(mesh_file_path, json_data);
  //      DOLAS_RETURN_FALSE_IF_FALSE(ret);

  //      RenderPrimitiveID primitive_id = HashConverter::StringHash(mesh_file_path);

  //      // 验证顶点数量
  //      if (!json_data.contains("vertex_count") || !json_data.contains("vertex_list"))
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: vertex_count or vertex_list not found in {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }

  //      int vertex_count = json_data["vertex_count"];
  //      const auto& vertex_list = json_data["vertex_list"];

  //      if (vertex_count != vertex_list.size())
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: vertex_count mismatch in {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }

  //      // 解析 IA
  //      if (!json_data.contains("input_layout"))
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: input_layout is Empty {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }
  //      const auto& input_layout = json_data["input_layout"];
  //      Bool has_position = false;
  //      Bool has_uv = false;
  //      Bool has_normal = false;
  //      int pos_element_count = 0;
  //      int uv_element_count = 0;
  //      int normal_element_count = 0;

  //      if (input_layout.contains("position"))
  //      {
  //          has_position = true;
  //          pos_element_count = input_layout["position"];
  //      }

  //      if (input_layout.contains("uv"))
  //      {
  //          has_uv = true;
  //          uv_element_count = input_layout["uv"];
  //      }

  //      if (input_layout.contains("normal"))
  //      {
  //          has_normal = true;
  //          normal_element_count = input_layout["normal"];
  //      }

  //      std::vector<std::vector<Float>> final_vertices;
  //      InputLayoutType input_layout_type = InputLayoutType::InputLayoutType_POS_3;

  //      if (has_position && has_uv && has_normal)
  //      {
  //          final_vertices.resize(3);
  //          if (pos_element_count == 3 && uv_element_count == 2 && normal_element_count == 3)
  //          {
  //              input_layout_type = InputLayoutType::InputLayoutType_POS_3_UV_2_NORM_3;
  //              for (const auto& vertex : vertex_list)
  //              {
  //                  const auto& pos = vertex["position"];
  //                  const auto& uv = vertex["uv"];
  //                  const auto& normal = vertex["normal"];

  //                  final_vertices[0].push_back(pos[0]);
  //                  final_vertices[0].push_back(pos[1]);
  //                  final_vertices[0].push_back(pos[2]);

  //                  final_vertices[1].push_back(uv[0]);
  //                  final_vertices[1].push_back(uv[1]);

  //                  final_vertices[2].push_back(normal[0]);
  //                  final_vertices[2].push_back(normal[1]);
  //                  final_vertices[2].push_back(normal[2]);
  //              }
  //          }
  //          else
  //          {
  //              LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
  //              return RENDER_PRIMITIVE_ID_EMPTY;
  //          }
  //      }
  //      else if (has_position && has_uv)
  //      {
  //          final_vertices.resize(2);
  //          if (pos_element_count == 3 && uv_element_count == 2)
  //          {
  //              input_layout_type = InputLayoutType::InputLayoutType_POS_3_UV_2;
  //              for (const auto& vertex : vertex_list)
  //              {
  //                  const auto& pos = vertex["position"];
  //                  const auto& uv = vertex["uv"];

  //                  final_vertices[0].push_back(pos[0]);
  //                  final_vertices[0].push_back(pos[1]);
  //                  final_vertices[0].push_back(pos[2]);

  //                  final_vertices[1].push_back(uv[0]);
  //                  final_vertices[1].push_back(uv[1]);
  //              }
  //          }
  //          else
  //          {
  //              LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
  //              return RENDER_PRIMITIVE_ID_EMPTY;
  //          }
  //      }
		//else if (has_position && has_normal)
		//{
		//	final_vertices.resize(2);
		//	if (pos_element_count == 3 && normal_element_count == 3)
		//	{
		//		input_layout_type = InputLayoutType::InputLayoutType_POS_3_NORM_3;
		//		for (const auto& vertex : vertex_list)
		//		{
		//			const auto& pos = vertex["position"];
		//			const auto& normal = vertex["normal"];

		//			final_vertices[0].push_back(pos[0]);
		//			final_vertices[0].push_back(pos[1]);
		//			final_vertices[0].push_back(pos[2]);

		//			final_vertices[1].push_back(normal[0]);
		//			final_vertices[1].push_back(normal[1]);
		//			final_vertices[1].push_back(normal[2]);
		//		}
		//	}
		//	else
		//	{
		//		LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported input_layout combination in {0}", mesh_file_path);
		//		return RENDER_PRIMITIVE_ID_EMPTY;
		//	}
		//}
		//else
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: input_layout missing required elements in {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }

  //      // 解析索引数据
  //      std::vector<UInt> indices;
  //      if (json_data.contains("index_count") && json_data.contains("index_list"))
  //      {
  //          int index_count = json_data["index_count"];
  //          const auto& index_list = json_data["index_list"];

  //          if (index_count != index_list.size())
  //          {
  //              LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: index_count mismatch in {0}", mesh_file_path);
  //              return RENDER_PRIMITIVE_ID_EMPTY;
  //          }

  //          indices.reserve(index_count);
  //          for (const auto& index : index_list)
  //          {
  //              indices.push_back(index);
  //          }
  //      }

  //      // 解析 Primitive
  //      if (!json_data.contains("primitive"))
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: primitive is Empty {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }
  //      const std::string& primitive_str = json_data["primitive"];
  //      PrimitiveTopology primitive_topology = PrimitiveTopology::PrimitiveTopology_TriangleList;
  //      if (primitive_str == "TriangleList")
  //      {
  //          primitive_topology = PrimitiveTopology::PrimitiveTopology_TriangleList;
  //      }
  //      else
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: unsupported primitive type in {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }

  //      Bool success = CreateRenderPrimitive(
  //          primitive_id,
  //          primitive_topology,
  //          input_layout_type,
  //          final_vertices,
  //          indices);
  //      if (!success)
  //      {
  //          LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitiveFromMeshFile: failed to create render primitive for {0}", mesh_file_path);
  //          return RENDER_PRIMITIVE_ID_EMPTY;
  //      }

        /*return primitive_id;*/
		return RENDER_PRIMITIVE_ID_EMPTY;
    }

	Bool RenderPrimitiveManager::InitializeSphereGeometry()
	{
		RenderPrimitiveManager* render_primitive_manager = g_dolas_engine.m_render_primitive_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_primitive_manager);

		std::vector<std::vector<Float>> vertices_data;
		std::vector<UInt> indices_data;
		if (!GenerateSphereRawData(20, vertices_data, indices_data))
		{
			LOG_ERROR("Failed to generate sphere Raw Data");
			return false;
		}
		RenderPrimitiveID sphere_render_primitive_string_id = STRING_ID(sphere_render_primitive);
		Bool success = render_primitive_manager->CreateRenderPrimitive(
			sphere_render_primitive_string_id,
			PrimitiveTopology::PrimitiveTopology_TriangleList,
			InputLayoutType::InputLayoutType_POS_3_NORM_3,
			vertices_data,
			indices_data);
		if (!success)
		{
			LOG_ERROR("Failed to CreateRenderPrimitive");
			return false;
		}

		m_base_geometries[BaseGeometryType_SPHERE] = sphere_render_primitive_string_id;
        return true;
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
        return true;
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
			InputLayoutType::InputLayoutType_POS_3_NORM_3,
			vertices_data,
			indices_data);
		if (!success)
		{
			LOG_ERROR("Failed to CreateRenderPrimitive");
			return false;
		}

		m_base_geometries[BaseGeometryType_CYLINDER] = cylinder_render_primitive_id;
        return true;
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
        return true;
	}

	Bool RenderPrimitiveManager::GenerateSphereRawData(UInt segments, std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices)
	{
		// 保证 segments 在合理范围内
		if (segments < 1 || segments > 100)
		{
			LOG_ERROR("Invalid segments value: {}, valid range is [1, 100]", segments);
			return false;
		}

		vertices_data.clear();
		indices.clear();

		// 使用双顶点流：
		//   stream 0: positions（x, y, z）
		//   stream 1: normals  （x, y, z）
		// 对应 InputLayoutType_POS_3_NORM_3
		vertices_data.resize(2);
		std::vector<Float>& positions = vertices_data[0];
		std::vector<Float>& normals   = vertices_data[1];

		const Float PI = 3.14159265358979323846f;
		const Float radius = 1.0f;

		// 划分段数
		UInt latitudeSegments = segments;
		UInt longitudeSegments = segments * 2;

		// 将一个小矩形面片拆成两个三角形，非 index 形式：每个三角形有独立的 3 个顶点
		auto add_vertex = [&](Float x, Float y, Float z)
		{
			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);

			// 单位球：位置归一化就是法线
			Float len_sq = x * x + y * y + z * z;
			if (len_sq > 0.0f)
			{
				Float inv_len = 1.0f / std::sqrt(len_sq);
				normals.push_back(x * inv_len);
				normals.push_back(y * inv_len);
				normals.push_back(z * inv_len);
			}
			else
			{
				// 退化时给个默认法线
				normals.push_back(0.0f);
				normals.push_back(1.0f);
				normals.push_back(0.0f);
			}
		};

		auto sphere_pos = [&](Float phi, Float theta)
		{
			Float sinPhi = std::sin(phi);
			Float cosPhi = std::cos(phi);
			Float sinTheta = std::sin(theta);
			Float cosTheta = std::cos(theta);

			Float x = radius * sinPhi * cosTheta;
			Float y = radius * cosPhi;
			Float z = radius * sinPhi * sinTheta;
			return std::tuple<Float, Float, Float>(x, y, z);
		};

		for (UInt lat = 0; lat < latitudeSegments; ++lat)
		{
			Float phi0 = static_cast<Float>(lat) / static_cast<Float>(latitudeSegments) * PI;
			Float phi1 = static_cast<Float>(lat + 1) / static_cast<Float>(latitudeSegments) * PI;

			for (UInt lon = 0; lon < longitudeSegments; ++lon)
			{
				Float theta0 = static_cast<Float>(lon) / static_cast<Float>(longitudeSegments) * 2.0f * PI;
				Float theta1 = static_cast<Float>(lon + 1) / static_cast<Float>(longitudeSegments) * 2.0f * PI;

				// 四个角点
				Float x00, y00, z00;
				std::tie(x00, y00, z00) = sphere_pos(phi0, theta0);
				Float x01, y01, z01;
				std::tie(x01, y01, z01) = sphere_pos(phi0, theta1);
				Float x10, y10, z10;
				std::tie(x10, y10, z10) = sphere_pos(phi1, theta0);
				Float x11, y11, z11;
				std::tie(x11, y11, z11) = sphere_pos(phi1, theta1);

				UInt base_index = static_cast<UInt>(positions.size() / 3);

				// 三角形 1: p00, p10, p01
				add_vertex(x00, y00, z00);
				add_vertex(x10, y10, z10);
				add_vertex(x01, y01, z01);

				// 三角形 2: p10, p11, p01
				add_vertex(x10, y10, z10);
				add_vertex(x11, y11, z11);
				add_vertex(x01, y01, z01);

				for (UInt k = 0; k < 6; ++k)
				{
					indices.push_back(base_index + k);
				}
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

		// 使用双顶点流：
		//   stream 0: positions（x, y, z）
		//   stream 1: normals  （x, y, z）
		// 对应 InputLayoutType_POS_3_NORM_3
		vertices_data.resize(2);
		std::vector<Float>& positions = vertices_data[0];
		std::vector<Float>& normals   = vertices_data[1];

		const Float radius = 1.0f;
		const Float half_height = 0.5f;
		const UInt  segments = 32;
		const Float PI = MathUtil::PI;

		auto add_vertex = [&](Float px, Float py, Float pz, Float nx, Float ny, Float nz)
		{
			positions.push_back(px);
			positions.push_back(py);
			positions.push_back(pz);

			// 归一化法线
			Float len_sq = nx * nx + ny * ny + nz * nz;
			if (len_sq > 0.0f)
			{
				Float inv_len = 1.0f / std::sqrt(len_sq);
				nx *= inv_len;
				ny *= inv_len;
				nz *= inv_len;
			}
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);
		};

		// ---------- 1. 侧面（圆柱侧壁）----------
		for (UInt i = 0; i < segments; ++i)
		{
			Float theta0 = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float theta1 = static_cast<Float>(i + 1) / static_cast<Float>(segments) * 2.0f * PI;

			Float cos0 = std::cos(theta0);
			Float sin0 = std::sin(theta0);
			Float cos1 = std::cos(theta1);
			Float sin1 = std::sin(theta1);

			// 四个角点
			Float x0 = radius * cos0;
			Float z0 = radius * sin0;
			Float x1 = radius * cos1;
			Float z1 = radius * sin1;

			Float yTop    = half_height;
			Float yBottom = -half_height;

			// 圆柱侧面法线 = 投影到 XZ 平面的归一化向量
			Float n0x = cos0, n0z = sin0;
			Float n1x = cos1, n1z = sin1;

			UInt base_index = static_cast<UInt>(positions.size() / 3);

			// 三角形 1: top_curr, bottom_curr, top_next
			add_vertex(x0, yTop,    z0, n0x, 0.0f, n0z);
			add_vertex(x0, yBottom, z0, n0x, 0.0f, n0z);
			add_vertex(x1, yTop,    z1, n1x, 0.0f, n1z);

			// 三角形 2: top_next, bottom_curr, bottom_next
			add_vertex(x1, yTop,    z1, n1x, 0.0f, n1z);
			add_vertex(x0, yBottom, z0, n0x, 0.0f, n0z);
			add_vertex(x1, yBottom, z1, n1x, 0.0f, n1z);

			for (UInt k = 0; k < 6; ++k)
			{
				indices.push_back(base_index + k);
			}
		}

		// ---------- 2. 顶盖 (+Y) ----------
		for (UInt i = 0; i < segments; ++i)
		{
			Float theta0 = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float theta1 = static_cast<Float>(i + 1) / static_cast<Float>(segments) * 2.0f * PI;

			Float cos0 = std::cos(theta0);
			Float sin0 = std::sin(theta0);
			Float cos1 = std::cos(theta1);
			Float sin1 = std::sin(theta1);

			Float y = half_height;

			// 顶盖法线朝 +Y
			const Float nx = 0.0f, ny = 1.0f, nz = 0.0f;

			Float cx = 0.0f, cz = 0.0f;
			Float x0 = radius * cos0, z0 = radius * sin0;
			Float x1 = radius * cos1, z1 = radius * sin1;

			UInt base_index = static_cast<UInt>(positions.size() / 3);

			// 三角形: center, p0, p1（顺时针为正面，根据你的 Cull 设置）
			add_vertex(cx, y, cz, nx, ny, nz);
			add_vertex(x0, y, z0, nx, ny, nz);
			add_vertex(x1, y, z1, nx, ny, nz);

			for (UInt k = 0; k < 3; ++k)
			{
				indices.push_back(base_index + k);
			}
		}

		// ---------- 3. 底盖 (-Y) ----------
		for (UInt i = 0; i < segments; ++i)
		{
			Float theta0 = static_cast<Float>(i) / static_cast<Float>(segments) * 2.0f * PI;
			Float theta1 = static_cast<Float>(i + 1) / static_cast<Float>(segments) * 2.0f * PI;

			Float cos0 = std::cos(theta0);
			Float sin0 = std::sin(theta0);
			Float cos1 = std::cos(theta1);
			Float sin1 = std::sin(theta1);

			Float y = -half_height;

			// 底盖法线朝 -Y
			const Float nx = 0.0f, ny = -1.0f, nz = 0.0f;

			Float cx = 0.0f, cz = 0.0f;
			Float x0 = radius * cos0, z0 = radius * sin0;
			Float x1 = radius * cos1, z1 = radius * sin1;

			UInt base_index = static_cast<UInt>(positions.size() / 3);

			// 为保持正面一致性，这里使用合适的三角形顶点顺序
			// 三角形: center, p1, p0
			add_vertex(cx, y, cz, nx, ny, nz);
			add_vertex(x1, y, z1, nx, ny, nz);
			add_vertex(x0, y, z0, nx, ny, nz);

			for (UInt k = 0; k < 3; ++k)
			{
				indices.push_back(base_index + k);
			}
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
		std::vector<Float>& normals   = vertices_data[1];

		const Float half_extent = 0.5f;

		// 先定义 8 个立方体顶点（用于查表），中心在原点，边长为 1（范围 [-0.5, 0.5]）
		struct Vec3
		{
			Float x, y, z;
		};
		Vec3 base_positions[8] =
		{
			{ -half_extent, -half_extent, -half_extent }, // v0
			{ -half_extent,  half_extent, -half_extent }, // v1
			{  half_extent,  half_extent, -half_extent }, // v2
			{  half_extent, -half_extent, -half_extent }, // v3
			{ -half_extent, -half_extent,  half_extent }, // v4
			{ -half_extent,  half_extent,  half_extent }, // v5
			{  half_extent,  half_extent,  half_extent }, // v6
			{  half_extent, -half_extent,  half_extent }  // v7
		};

		// 每个三角形单独写入 3 个顶点（非 index 形式）
		// indices 数组就是 0,1,2,3,...,35
		auto add_triangle = [&](UInt i0, UInt i1, UInt i2, const Vec3& normal)
		{
			UInt base_index = static_cast<UInt>(positions.size() / 3);

			auto push_vertex = [&](UInt vi)
			{
				const Vec3& p = base_positions[vi];
				positions.push_back(p.x);
				positions.push_back(p.y);
				positions.push_back(p.z);

				normals.push_back(normal.x);
				normals.push_back(normal.y);
				normals.push_back(normal.z);
			};

			push_vertex(i0);
			push_vertex(i1);
			push_vertex(i2);

			indices.push_back(base_index + 0);
			indices.push_back(base_index + 1);
			indices.push_back(base_index + 2);
		};

		// 六个面的法线
		const Vec3 n_front  = {  0.0f,  0.0f,  1.0f };
		const Vec3 n_back   = {  0.0f,  0.0f, -1.0f };
		const Vec3 n_left   = { -1.0f,  0.0f,  0.0f };
		const Vec3 n_right  = {  1.0f,  0.0f,  0.0f };
		const Vec3 n_top    = {  0.0f,  1.0f,  0.0f };
		const Vec3 n_bottom = {  0.0f, -1.0f,  0.0f };

		// 6 个面，每个面 2 个三角形，共 12 个三角形（36 个顶点 / 索引）
		// 前面 (+Z)：v4, v5, v6, v7
		add_triangle(4, 5, 6, n_front);
		add_triangle(4, 6, 7, n_front);

		// 后面 (-Z)：v0, v1, v2, v3
		add_triangle(3, 2, 1, n_back);
		add_triangle(3, 1, 0, n_back);

		// 左面 (-X)：v0, v1, v5, v4
		add_triangle(0, 1, 5, n_left);
		add_triangle(0, 5, 4, n_left);

		// 右面 (+X)：v3, v2, v6, v7
		add_triangle(7, 6, 2, n_right);
		add_triangle(7, 2, 3, n_right);

		// 顶面 (+Y)：v1, v2, v6, v5
		add_triangle(1, 2, 6, n_top);
		add_triangle(1, 6, 5, n_top);

		// 底面 (-Y)：v0, v4, v7, v3
		add_triangle(0, 4, 7, n_bottom);
		add_triangle(0, 7, 3, n_bottom);

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
        for (std::size_t stream_index = 0; stream_index < vertices.size(); stream_index++)
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

            const std::size_t vc = (vertex_stride == 0) ? 0 : (vertices[stream_index].size() / vertex_stride);
            if (vc > (std::size_t)(std::numeric_limits<UInt>::max)())
            {
                LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: vertex_count overflow ({})", vc);
                return nullptr;
            }
		    vertex_count = (UInt)vc;

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
        const std::size_t ic = indices.size();
        if (ic > (std::size_t)(std::numeric_limits<UInt>::max)())
        {
            LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: index_count overflow ({})", ic);
            return nullptr;
        }
        UInt index_count = (UInt)ic;

        const std::size_t index_bytes_sz = ic * sizeof(UInt);
        if (index_bytes_sz > (std::size_t)(std::numeric_limits<std::uint32_t>::max)())
        {
            LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: index buffer size overflow ({})", index_bytes_sz);
            return nullptr;
        }
        BufferID index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer((std::uint32_t)index_bytes_sz, indices.data());
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



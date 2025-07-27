#include <iostream>
#include <fstream>

#include "core/dolas_engine.h"
#include "manager/dolas_mesh_manager.h"
#include "render/dolas_mesh.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "nlohmann/json.hpp"
#include "manager/dolas_asset_manager.h"
using json = nlohmann::json;

namespace Dolas
{
    MeshManager::MeshManager()
    {

    }

    MeshManager::~MeshManager()
    {

    }
    
    bool MeshManager::Initialize()
    {
        return true;
    }

    bool MeshManager::Clear()
    {
        for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
        {
            Mesh* mesh = it->second;
            if (mesh)
            {
                // 清理网格数据
                mesh->m_vertices.clear();
                mesh->m_uvs.clear();
                mesh->m_normals.clear();
                mesh->m_tangents.clear();
                mesh->m_bitangents.clear();
                mesh->m_indices.clear();
            }
        }
        m_meshes.clear();
        return true;
    }

    MeshID MeshManager::CreateMesh(const std::string& mesh_file_name)
    {
        std::string mesh_file_path = PathUtils::GetMeshDir() + mesh_file_name;

        json json_data;
        Bool ret = g_dolas_engine.m_asset_manager->LoadJsonFile(mesh_file_path, json_data);
        DOLAS_RETURN_FALSE_IF_FALSE(ret);

        // 创建网格对象
        Mesh* mesh = DOLAS_NEW(Mesh);
        mesh->m_file_id = STRING_ID(mesh_file_path);

        // 验证顶点数量
        if (!json_data.contains("vertex_count") || !json_data.contains("vertex_list"))
        {
            std::cerr << "MeshManager::CreateMesh: vertex_count or vertex_list not found in " << mesh_file_path << std::endl;
            return MESH_ID_EMPTY;
        }

        int vertex_count = json_data["vertex_count"];
        const auto& vertex_list = json_data["vertex_list"];

        if (vertex_count != vertex_list.size())
        {
            std::cerr << "MeshManager::CreateMesh: vertex_count mismatch in " << mesh_file_path << std::endl;
            return MESH_ID_EMPTY;
        }

        // 解析顶点数据
        mesh->m_vertices.reserve(vertex_count);
        mesh->m_uvs.reserve(vertex_count);

        for (const auto& vertex : vertex_list)
        {
            if (vertex.contains("position"))
            {
                const auto& pos = vertex["position"];
                if (pos.size() >= 3)
                {
                    mesh->m_vertices.emplace_back(pos[0], pos[1], pos[2]);
                }
                else
                {
                    std::cerr << "MeshManager::CreateMesh: invalid position data in " << mesh_file_path << std::endl;
                    return MESH_ID_EMPTY;
                }
            }
            else
            {
                std::cerr << "MeshManager::CreateMesh: position not found for vertex in " << mesh_file_path << std::endl;
                return MESH_ID_EMPTY;
            }

            if (vertex.contains("uv"))
            {
                const auto& uv = vertex["uv"];
                if (uv.size() >= 2)
                {
                    mesh->m_uvs.emplace_back(uv[0], uv[1]);
                }
                else
                {
                    std::cerr << "MeshManager::CreateMesh: invalid uv data in " << mesh_file_path << std::endl;
                    return MESH_ID_EMPTY;
                }
            }
            else
            {
                // UV坐标是可选的，如果没有就设置为默认值
                mesh->m_uvs.emplace_back(0.0f, 0.0f);
            }

			if (vertex.contains("normal"))
			{
				const auto& normal = vertex["normal"];
				if (normal.size() >= 3)
				{
					mesh->m_normals.emplace_back(normal[0], normal[1], normal[2]);
				}
				else
				{
					std::cerr << "MeshManager::CreateMesh: invalid normal data in " << mesh_file_path << std::endl;
					return MESH_ID_EMPTY;
				}
			}
			else
			{
				std::cerr << "MeshManager::CreateMesh: normal not found for vertex in " << mesh_file_path << std::endl;
				return MESH_ID_EMPTY;
			}
        }

        // 解析索引数据
        if (json_data.contains("index_count") && json_data.contains("index_list"))
        {
            int index_count = json_data["index_count"];
            const auto& index_list = json_data["index_list"];

            if (index_count != index_list.size())
            {
                std::cerr << "MeshManager::CreateMesh: index_count mismatch in " << mesh_file_path << std::endl;
                return MESH_ID_EMPTY;
            }

            mesh->m_indices.reserve(index_count);
            for (const auto& index : index_list)
            {
                mesh->m_indices.push_back(index);
            }
        }

        // 合并到最终的大缓冲区
        for (int i = 0; i < mesh->m_vertices.size(); i++)
        {
            mesh->m_final_vertices.push_back(mesh->m_vertices[i].x);
            mesh->m_final_vertices.push_back(mesh->m_vertices[i].y);
            mesh->m_final_vertices.push_back(mesh->m_vertices[i].z);
            mesh->m_final_vertices.push_back(mesh->m_uvs[i].x);
            mesh->m_final_vertices.push_back(mesh->m_uvs[i].y);
			mesh->m_final_vertices.push_back(mesh->m_normals[i].x);
			mesh->m_final_vertices.push_back(mesh->m_normals[i].y);
			mesh->m_final_vertices.push_back(mesh->m_normals[i].z);
        }

        m_meshes[mesh->m_file_id] = mesh;
        return mesh->m_file_id;
    }

    Mesh* MeshManager::GetMesh(MeshID mesh_id)
    {
        if (m_meshes.find(mesh_id) == m_meshes.end())
        {
            return nullptr;
        }
        return m_meshes[mesh_id];
    }
}
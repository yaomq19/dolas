#include "manager/dolas_mesh_manager.h"
#include "render/dolas_mesh.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>

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

    Mesh* MeshManager::GetOrCreateMesh(const std::string& file_name)
    {
        if (m_meshes.find(file_name) == m_meshes.end())
        {
            Mesh* mesh = CreateMesh(file_name);
            if (mesh != nullptr)
            {
                m_meshes[file_name] = mesh;
            }
            return mesh;
        }
        return m_meshes[file_name];
    }

    Mesh* MeshManager::CreateMesh(const std::string& file_name)
    {
        json json_data;
        std::string mesh_path = PathUtils::GetMeshDir() + file_name;

        // 读取JSON文件
        std::ifstream file(mesh_path);
        if (!file.is_open())
        {
            std::cerr << "MeshManager::CreateMesh: file is not found in " << mesh_path << std::endl;
            return nullptr;
        }

        try 
        {
            file >> json_data;
            file.close();
        }
        catch (const json::parse_error& e)
        {
            std::cerr << "MeshManager::CreateMesh: JSON parse error in " << mesh_path << ": " << e.what() << std::endl;
            file.close();
            return nullptr;
        }

        // 创建网格对象
        Mesh* mesh = DOLAS_NEW(Mesh);
        mesh->m_file_path = mesh_path;

        // 验证顶点数量
        if (!json_data.contains("vertex_count") || !json_data.contains("vertex_list"))
        {
            std::cerr << "MeshManager::CreateMesh: vertex_count or vertex_list not found in " << mesh_path << std::endl;
            return nullptr;
        }

        int vertex_count = json_data["vertex_count"];
        const auto& vertex_list = json_data["vertex_list"];

        if (vertex_count != vertex_list.size())
        {
            std::cerr << "MeshManager::CreateMesh: vertex_count mismatch in " << mesh_path << std::endl;
            return nullptr;
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
                    std::cerr << "MeshManager::CreateMesh: invalid position data in " << mesh_path << std::endl;
                    return nullptr;
                }
            }
            else
            {
                std::cerr << "MeshManager::CreateMesh: position not found for vertex in " << mesh_path << std::endl;
                return nullptr;
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
                    std::cerr << "MeshManager::CreateMesh: invalid uv data in " << mesh_path << std::endl;
                    return nullptr;
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
					std::cerr << "MeshManager::CreateMesh: invalid normal data in " << mesh_path << std::endl;
					return nullptr;
				}
			}
			else
			{
				std::cerr << "MeshManager::CreateMesh: normal not found for vertex in " << mesh_path << std::endl;
				return nullptr;
			}
        }

        // 解析索引数据
        if (json_data.contains("index_count") && json_data.contains("index_list"))
        {
            int index_count = json_data["index_count"];
            const auto& index_list = json_data["index_list"];

            if (index_count != index_list.size())
            {
                std::cerr << "MeshManager::CreateMesh: index_count mismatch in " << mesh_path << std::endl;
                return nullptr;
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
        std::cout << "MeshManager::CreateMesh: Successfully created mesh from " << mesh_path << std::endl;
        std::cout << "  Vertices: " << mesh->m_vertices.size() << std::endl;
        std::cout << "  UVs: " << mesh->m_uvs.size() << std::endl;
        std::cout << "  Indices: " << mesh->m_indices.size() << std::endl;

        return mesh;
    }
}
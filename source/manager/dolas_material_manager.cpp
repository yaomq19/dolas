#include "core/dolas_engine.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "nlohmann/json.hpp"
#include "base/dolas_dx_trace.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

namespace Dolas
{
    MaterialManager::MaterialManager()
    {

    }

    MaterialManager::~MaterialManager()
    {

    }

    bool MaterialManager::Initialize()
    {
        return true;
    }

    bool MaterialManager::Clear()
    {
        for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
        {
            Material* material = it->second;
            if (material)
            {
                // 清理材质资源
                if (material->m_vertex_shader)
                {
                    material->m_vertex_shader->Release();
                    material->m_vertex_shader = nullptr;
                }
                if (material->m_pixel_shader)
                {
                    material->m_pixel_shader->Release();
                    material->m_pixel_shader = nullptr;
                }
            }
        }
        m_materials.clear();
        return true;
    }

    Material* MaterialManager::GetOrCreateMaterial(const std::string& file_name)
    {
        if (m_materials.find(file_name) == m_materials.end())
        {
            Material* material = CreateMaterial(file_name);
            if (material != nullptr)
            {
                m_materials[file_name] = material;
            }
        }
        return m_materials[file_name];
    }

    Material* MaterialManager::CreateMaterial(const std::string& file_name)
    {
        json json_data;
        std::string material_path = PathUtils::GetMaterialDir() + file_name;

        // 读取JSON文件
        std::ifstream file(material_path);
        if (!file.is_open())
        {
            std::cerr << "MaterialManager::CreateMaterial: file is not found in " << material_path << std::endl;
            return nullptr;
        }

        try 
        {
            file >> json_data;
            file.close();
        }
        catch (const json::parse_error& e)
        {
            std::cerr << "MaterialManager::CreateMaterial: JSON parse error in " << material_path << ": " << e.what() << std::endl;
            file.close();
            return nullptr;
        }

        // 创建材质对象
        Material* material = DOLAS_NEW(Material);
        material->m_file_path = material_path;

        // 顶点着色器
        if (json_data.contains("vertex_shader"))
        {
            material->m_vertex_shader = g_dolas_engine.m_shader_manager->GetOrCreateVertexShader(json_data["vertex_shader"], "VS");
        }

        // 像素着色器
        if (json_data.contains("pixel_shader"))
        {
            material->m_pixel_shader = g_dolas_engine.m_shader_manager->GetOrCreatePixelShader(json_data["pixel_shader"], "PS");
        }

        // 解析纹理信息
        if (json_data.contains("texture"))
        {
            const auto& textures = json_data["texture"];
            for (auto it = textures.begin(); it != textures.end(); ++it)
            {
                std::string texture_name = it.key();
                std::string texture_file = it.value();
                
                // 根据纹理名称分配插槽
                int slot = 0;
                if (texture_name == "albedo_map") slot = 0;
                else if (texture_name == "normal_map") slot = 1;
                else if (texture_name == "roughness_map") slot = 2;
                else if (texture_name == "metallic_map") slot = 3;
                // 可以扩展更多纹理类型
                
                material->m_textures.push_back(std::make_pair(slot, texture_file));
                std::cout << "Material texture [" << texture_name << "]: " << texture_file << " -> slot " << slot << std::endl;
            }
        }

        // 解析参数信息
        if (json_data.contains("parameter"))
        {
            const auto& parameters = json_data["parameter"];
            for (auto it = parameters.begin(); it != parameters.end(); ++it)
            {
                std::string param_name = it.key();
                auto param_value = it.value();
                
                // TODO: 在这里处理材质参数
                // 可能需要扩展Material类来存储这些参数
                std::cout << "Material parameter [" << param_name << "]: " << param_value << std::endl;
            }
        }

        std::cout << "MaterialManager::CreateMaterial: Successfully created material from " << material_path << std::endl;
        return material;
    }
    
} // namespace Dolas
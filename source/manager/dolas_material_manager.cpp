#include <iostream>
#include <fstream>

#include "core/dolas_engine.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "nlohmann/json.hpp"
#include "base/dolas_dx_trace.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_texture_manager.h"

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
        InitializeGlobalMaterial();
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

    void MaterialManager::InitializeGlobalMaterial()
    {
        m_global_materials.m_deferred_shading = CreateMaterial("deferred_shading.material");
    }

    MaterialID MaterialManager::CreateMaterial(const std::string& file_name)
    {
        std::string material_file_path = PathUtils::GetMaterialDir() + file_name;

        json json_data;
        Bool ret = g_dolas_engine.m_asset_manager->LoadJsonFile(material_file_path, json_data);
        if (!ret)
        {
            return MATERIAL_ID_EMPTY;
        }

        // 创建材质对象
        Material* material = DOLAS_NEW(Material);
        material->m_file_id = HashConverter::StringHash(material_file_path);
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
		if (json_data.contains("vertex_shader_texture"))
		{
			const auto& textures = json_data["vertex_shader_texture"];
			for (auto it = textures.begin(); it != textures.end(); ++it)
			{
				std::string texture_name = it.key();
				std::string texture_file_name = it.value();

				TextureID texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromFile(texture_file_name);
				if (texture_id == TEXTURE_ID_EMPTY)
				{
					return MATERIAL_ID_EMPTY;
				}

				// 根据纹理名称分配插槽
				int slot = 0;
				if (texture_name == "albedo_map") slot = 0;
				else if (texture_name == "normal_map") slot = 1;
				else if (texture_name == "roughness_map") slot = 2;
				else if (texture_name == "metallic_map") slot = 3;
				// 可以扩展更多纹理类型

				material->m_vertex_shader_textures[slot] = texture_id;
			}
		}

        if (json_data.contains("pixel_shader_texture"))
        {
            const auto& textures = json_data["pixel_shader_texture"];
            for (auto it = textures.begin(); it != textures.end(); ++it)
            {
                std::string texture_name = it.key();
                std::string texture_file_name = it.value();
                
                TextureID texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromFile(texture_file_name);
                if (texture_id == TEXTURE_ID_EMPTY)
                {
                    return MATERIAL_ID_EMPTY;
                }

                // 根据纹理名称分配插槽
                int slot = 0;
                if (texture_name == "albedo_map") slot = 0;
                else if (texture_name == "normal_map") slot = 1;
                else if (texture_name == "roughness_map") slot = 2;
                else if (texture_name == "metallic_map") slot = 3;
                // 可以扩展更多纹理类型
                
                material->m_pixel_shader_textures[slot] = texture_id;
            }
        }

        // 解析参数信息
        if (json_data.contains("parameter"))
        {
            const auto& parameters = json_data["parameter"];
            for (auto it = parameters.begin(); it != parameters.end(); ++it)
            {
            }
        }

        m_materials[material->m_file_id] = material;
        return material->m_file_id;
    }

    Material* MaterialManager::GetMaterial(MaterialID material_id)
    {
        if (m_materials.find(material_id) == m_materials.end())
        {
            return nullptr;
        }
        return m_materials[material_id];
    }

    MaterialID MaterialManager::GetDeferredShadingMaterialID()
    {
        return m_global_materials.m_deferred_shading;
    }
    
} // namespace Dolas
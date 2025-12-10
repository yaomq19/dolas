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
#include "manager/dolas_log_system_manager.h"
using json = nlohmann::json;

namespace Dolas
{
    MaterialManager::MaterialManager()
    {

    }

    MaterialManager::~MaterialManager()
    {
        Clear();
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
                // Material 持有的是共享的 ShaderContext，不负责销毁底层 D3D 资源
                material->m_vertex_context.reset();
                material->m_pixel_context.reset();
                // 释放 Material 本身
                DOLAS_DELETE(material);
            }
        }
        m_materials.clear();
        return true;
    }

    void MaterialManager::InitializeGlobalMaterial()
    {
        m_global_materials.m_deferred_shading = CreateMaterial("deferred_shading.material");
        m_global_materials.m_sky_box_material_id = CreateMaterial("sky_box.material");
        m_global_materials.m_debug_draw_material_id = CreateMaterial("debug_draw.material");
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
            material->m_vertex_context = CreateVertexContext(json_data["vertex_shader"], "VS");
        }

        // 像素着色器
        if (json_data.contains("pixel_shader"))
        {
            material->m_pixel_context = CreatePixelContext(json_data["pixel_shader"], "PS");
        }

        // 解析纹理信息
		if (json_data.contains("vertex_shader_texture"))
		{
			const auto& textures = json_data["vertex_shader_texture"];
			for (auto it = textures.begin(); it != textures.end(); ++it)
			{
				std::string texture_name = it.key();
				std::string texture_file_name = it.value();

				TextureID texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromDDSFile(texture_file_name);
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
                material->m_vertex_context->SetShaderResourceView(slot, texture_id);
			}
		}

        if (json_data.contains("pixel_shader_texture"))
        {
            const auto& textures = json_data["pixel_shader_texture"];
            for (auto it = textures.begin(); it != textures.end(); ++it)
            {
                std::string texture_name = it.key();
                std::string texture_file_name = it.value();
                
                TextureID texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromDDSFile(texture_file_name);
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
                material->m_pixel_context->SetShaderResourceView(slot, texture_id);
            }
        }

        // 解析全局常量缓冲（GlobalConstants）参数
        // 约定：vertex_shader_global_variables / pixel_shader_global_variables 为
        // { "变量名": [f0, f1, f2, f3 ...] } 形式的对象。
        if (json_data.contains("vertex_shader_global_variables") && material->m_vertex_context)
        {
            const auto& parameters = json_data["vertex_shader_global_variables"];
            if (parameters.is_object())
            {
                for (auto it = parameters.begin(); it != parameters.end(); ++it)
                {
                    const std::string var_name = it.key();
                    const auto& value_array = it.value();
                    if (!value_array.is_array() || value_array.size() != 4)
                    {
                        LOG_ERROR("!value_array.is_array() || value_array.size() != 4");
                        continue;
                    }

                    Int idx = 0;
					Vector4 values;
                    for (const auto& v : value_array)
                    {
                        values[idx++] = v.get<float>();
                    }

                    material->m_vertex_context->SetGlobalVariable(var_name, values);
                }
            }
        }

        if (json_data.contains("pixel_shader_global_variables") && material->m_pixel_context)
        {
            const auto& parameters = json_data["pixel_shader_global_variables"];
            if (parameters.is_object())
            {
                for (auto it = parameters.begin(); it != parameters.end(); ++it)
                {
                    const std::string var_name = it.key();
                    const auto& value_array = it.value();
                    if (!value_array.is_array() || value_array.size() != 4)
                    {
                        LOG_ERROR("!value_array.is_array() || value_array.size() != 4");
                        continue;
                    }

                    Int idx = 0;
                    Vector4 values;
                    for (const auto& v : value_array)
                    {
                        values[idx++] = v.get<float>();
                    }

                    material->m_pixel_context->SetGlobalVariable(var_name, values);
                }
            }
        }

        m_materials[material->m_file_id] = material;
        return material->m_file_id;
    }

    Material* MaterialManager::GetMaterialByID(MaterialID material_id)
    {
        if (m_materials.find(material_id) == m_materials.end())
        {
            return nullptr;
        }
        return m_materials[material_id];
    }

    Material* MaterialManager::GetDeferredShadingMaterial()
    {
        return GetMaterialByID(m_global_materials.m_deferred_shading);
    }
    
    Material* MaterialManager::GetSkyBoxMaterial()
    {
        return GetMaterialByID(m_global_materials.m_sky_box_material_id);
    }

    Material* MaterialManager::GetDebugDrawMaterial()
    {
		return GetMaterialByID(m_global_materials.m_debug_draw_material_id);
    }

	// protected methods
    std::shared_ptr<VertexContext> MaterialManager::CreateVertexContext(const std::string& file_path, const std::string& entry_point)
    {
        // 通过 ShaderManager 复用/创建底层 VertexContext
        return g_dolas_engine.m_shader_manager->GetOrCreateVertexContext(file_path, entry_point);
    }

    std::shared_ptr<PixelContext> MaterialManager::CreatePixelContext(const std::string& file_path, const std::string& entry_point)
    {
        return g_dolas_engine.m_shader_manager->GetOrCreatePixelContext(file_path, entry_point);
    }
} // namespace Dolas
#include <iostream>
#include <fstream>

#include "core/dolas_engine.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "base/dolas_dx_trace.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "rsd/material.h"

namespace Dolas
{
    DOLAS_STATIC_CONST std::string k_global_material_directory = "global_material/";

    DOLAS_STATIC_CONST std::array<std::string, static_cast<UInt>(GlobalMaterialType::Count)>
        k_global_material_file_names = {
            "deferred_shading.material",
            "sky_box.material",
            "debug_draw.material"
        };

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
        m_global_materials[(UInt)GlobalMaterialType::DeferredShading] = CreateMaterial(k_global_material_directory + k_global_material_file_names[(UInt)GlobalMaterialType::DeferredShading]);
        m_global_materials[(UInt)GlobalMaterialType::SkyBox] = CreateMaterial(k_global_material_directory + k_global_material_file_names[(UInt)GlobalMaterialType::SkyBox]);
        m_global_materials[(UInt)GlobalMaterialType::DebugDraw] = CreateMaterial(k_global_material_directory + k_global_material_file_names[(UInt)GlobalMaterialType::DebugDraw]);
    }

    MaterialID MaterialManager::CreateMaterial(const std::string& file_name)
    {
        std::string material_file_path = PathUtils::GetContentDir() + file_name;

        // 其他系统不需要知道 XML：统一通过 RSD 资产读取
        MaterialRSD* material_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<MaterialRSD>(file_name);
        if (material_rsd == nullptr)
            return MATERIAL_ID_EMPTY;

        // 创建材质对象
        Material* material = DOLAS_NEW(Material);
        material->m_file_id = HashConverter::StringHash(material_file_path);
        // 顶点着色器
        if (!material_rsd->vertex_shader.empty())
            material->m_vertex_context = CreateVertexContext(material_rsd->vertex_shader, "VS");

        // 像素着色器
        if (!material_rsd->pixel_shader.empty())
            material->m_pixel_context = CreatePixelContext(material_rsd->pixel_shader, "PS");

        // 纹理（目前只做 pixel_shader_texture，跟你现有 content 对齐）
        if (material->m_pixel_context)
        {
            for (const auto& kv : material_rsd->pixel_shader_texture)
            {
                const std::string& texture_name = kv.first;
                const std::string& texture_file_name = kv.second;

				TextureID texture_id = TEXTURE_ID_EMPTY;

				if (texture_file_name.ends_with(".dds") || texture_file_name.ends_with(".DDS"))
                {
                    texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromDDSFile(texture_file_name);
                }
                else if (texture_file_name.ends_with(".png") || texture_file_name.ends_with(".PNG"))
                {
                    texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromPNGFile(texture_file_name);
                }
                else
                {
                    LOG_ERROR("Unsupported texture format for material: {0}, texture: {1}", file_name, texture_file_name);
                    continue;
				}

                int slot = 0;
                if (texture_name == "albedo_map") slot = 0;
                else if (texture_name == "normal_map") slot = 1;
                else if (texture_name == "roughness_map") slot = 2;
                else if (texture_name == "metallic_map") slot = 3;

                material->m_pixel_context->SetShaderResourceView(slot, texture_id);
            }
        }

        // 全局变量
        if (material->m_vertex_context)
        {
            for (const auto& kv : material_rsd->vertex_shader_global_variables)
                material->m_vertex_context->SetGlobalVariable(kv.first, kv.second);
        }
        if (material->m_pixel_context)
        {
            for (const auto& kv : material_rsd->pixel_shader_global_variables)
                material->m_pixel_context->SetGlobalVariable(kv.first, kv.second);
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

	Dolas::Material* MaterialManager::GetGlobalMaterial(GlobalMaterialType global_material_type)
	{
		return GetMaterialByID(m_global_materials[static_cast<UInt>(global_material_type)]);
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
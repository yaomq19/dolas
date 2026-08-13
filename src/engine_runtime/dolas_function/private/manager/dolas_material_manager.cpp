#include <iostream>
#include <fstream>
#include <string_view>

#include "dolas_engine.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_material.h"
#include "dolas_base.h"
#include "render/dolas_dx_trace.h"
#include "dolas_asset_path.h"
#include "dolas_asset_manager.h"
#include "manager/dolas_texture_manager.h"
#include "dolas_log_system_manager.h"
#include "rsd/material.h"

namespace Dolas
{
    DOLAS_STATIC_CONST std::array<std::string, static_cast<UInt>(GlobalMaterialType::Count)>
        k_global_material_asset_paths = {
            "_engine/global_material/deferred_shading.material",
            "_engine/global_material/sky_box.material",
            "_engine/global_material/debug_draw.material"
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
        for (UInt index = 0; index < static_cast<UInt>(GlobalMaterialType::Count); ++index)
        {
            const auto asset_path = AssetPath::Parse(k_global_material_asset_paths[index]);
            if (!asset_path)
            {
                LOG_ERROR("Invalid global material asset path: {0}", k_global_material_asset_paths[index]);
                m_global_materials[index] = MATERIAL_ID_EMPTY;
                continue;
            }

            m_global_materials[index] = CreateMaterial(*asset_path);
        }
    }

    MaterialID MaterialManager::CreateMaterial(const AssetPath& asset_path)
    {
        // 其他系统不需要知道 XML：统一通过 RSD 资产读取
        const MaterialRSD* material_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<MaterialRSD>(asset_path);
        if (material_rsd == nullptr)
            return MATERIAL_ID_EMPTY;

        // 创建材质对象
        Material* material = DOLAS_NEW(Material);
        material->m_file_id = HashConverter::StringHash(asset_path.GetCanonicalPath());
        // 顶点着色器
        if (!material_rsd->vertex_shader.empty())
        {
            const auto vertex_shader_path = AssetPath::Parse(material_rsd->vertex_shader);
            if (vertex_shader_path)
            {
                material->m_vertex_context = CreateVertexContext(*vertex_shader_path, "VS");
            }
            else
            {
                LOG_ERROR("Invalid vertex shader asset path in material {0}: {1}", asset_path.GetCanonicalPath(), material_rsd->vertex_shader);
            }
        }

        // 像素着色器
        if (!material_rsd->pixel_shader.empty())
        {
            const auto pixel_shader_path = AssetPath::Parse(material_rsd->pixel_shader);
            if (pixel_shader_path)
            {
                material->m_pixel_context = CreatePixelContext(*pixel_shader_path, "PS");
            }
            else
            {
                LOG_ERROR("Invalid pixel shader asset path in material {0}: {1}", asset_path.GetCanonicalPath(), material_rsd->pixel_shader);
            }
        }

        // 纹理（目前只做 pixel_shader_texture，跟你现有 content 对齐）
        if (material->m_pixel_context)
        {
            for (const auto& kv : material_rsd->pixel_shader_texture)
            {
                const std::string& texture_name = kv.first;
                const std::string& texture_file_name = kv.second;

				TextureID texture_id = TEXTURE_ID_EMPTY;
				const auto texture_asset_path = AssetPath::Parse(texture_file_name);
				if (!texture_asset_path)
				{
					LOG_ERROR("Invalid texture asset path in material {0}: {1}", asset_path.GetCanonicalPath(), texture_file_name);
					continue;
				}

				const std::string_view relative_path = texture_asset_path->GetRelativePath();
				if (relative_path.ends_with(".dds") || relative_path.ends_with(".DDS"))
                {
                    texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromDDSFile(*texture_asset_path);
                }
                else if (relative_path.ends_with(".png") || relative_path.ends_with(".PNG"))
                {
                    texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromPNGFile(*texture_asset_path);
                }
                else
                {
                    LOG_ERROR("Unsupported texture format for material: {0}, texture: {1}", asset_path.GetCanonicalPath(), texture_file_name);
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
        auto it = m_materials.find(material_id);
        return (it != m_materials.end()) ? it->second : nullptr;
    }

	Dolas::Material* MaterialManager::GetGlobalMaterial(GlobalMaterialType global_material_type)
	{
		return GetMaterialByID(m_global_materials[static_cast<UInt>(global_material_type)]);
	}

	// protected methods
    std::shared_ptr<VertexContext> MaterialManager::CreateVertexContext(const AssetPath& asset_path, const std::string& entry_point)
    {
        // 通过 ShaderManager 复用/创建底层 VertexContext
        return g_dolas_engine.m_shader_manager->GetOrCreateVertexContext(asset_path, entry_point);
    }

    std::shared_ptr<PixelContext> MaterialManager::CreatePixelContext(const AssetPath& asset_path, const std::string& entry_point)
    {
        return g_dolas_engine.m_shader_manager->GetOrCreatePixelContext(asset_path, entry_point);
    }
} // namespace Dolas

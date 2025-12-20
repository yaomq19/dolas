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
#include "tinyxml2.h"

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

        tinyxml2::XMLDocument doc;
        Bool ret = g_dolas_engine.m_asset_manager->LoadXmlFile(material_file_path, doc);
        if (!ret)
        {
            return MATERIAL_ID_EMPTY;
        }

        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root)
            return MATERIAL_ID_EMPTY;

        auto child_text = [&](const char* name) -> std::string
        {
            const tinyxml2::XMLElement* el = root->FirstChildElement(name);
            const char* t = el ? el->GetText() : nullptr;
            return t ? std::string(t) : std::string();
        };

        // 创建材质对象
        Material* material = DOLAS_NEW(Material);
        material->m_file_id = HashConverter::StringHash(material_file_path);
        // 顶点着色器
        {
            const auto vs = child_text("vertex_shader");
            if (!vs.empty())
                material->m_vertex_context = CreateVertexContext(vs, "VS");
        }

        // 像素着色器
        {
            const auto ps = child_text("pixel_shader");
            if (!ps.empty())
                material->m_pixel_context = CreatePixelContext(ps, "PS");
        }

        // 解析纹理信息
        auto bind_texture_list = [&](const char* listName, const auto& ctx)
        {
            if (!ctx) return;
            const tinyxml2::XMLElement* list = root->FirstChildElement(listName);
            if (!list) return;
            for (auto* tex = list->FirstChildElement("texture"); tex; tex = tex->NextSiblingElement("texture"))
            {
                const char* texture_name = tex->Attribute("name");
                const char* texture_file_name = tex->Attribute("file");
                if (!texture_name || !texture_file_name) continue;

                TextureID texture_id = g_dolas_engine.m_texture_manager->CreateTextureFromDDSFile(texture_file_name);
                if (texture_id == TEXTURE_ID_EMPTY)
                    continue;

                int slot = 0;
                std::string tn = texture_name;
                if (tn == "albedo_map") slot = 0;
                else if (tn == "normal_map") slot = 1;
                else if (tn == "roughness_map") slot = 2;
                else if (tn == "metallic_map") slot = 3;
                ctx->SetShaderResourceView(slot, texture_id);
            }
        };

        bind_texture_list("vertex_shader_texture", material->m_vertex_context);
        bind_texture_list("pixel_shader_texture", material->m_pixel_context);

        // 解析全局常量缓冲（GlobalConstants）参数
        auto bind_globals = [&](const char* blockName, const auto& ctx)
        {
            if (!ctx) return;
            const tinyxml2::XMLElement* block = root->FirstChildElement(blockName);
            if (!block) return;
            for (auto* v = block->FirstChildElement("vec4"); v; v = v->NextSiblingElement("vec4"))
            {
                const char* var_name = v->Attribute("name");
                if (!var_name) continue;
                double x = 0, y = 0, z = 0, w = 0;
                if (v->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("w", &w) != tinyxml2::XML_SUCCESS) continue;
                Vector4 values((Float)x, (Float)y, (Float)z, (Float)w);
                ctx->SetGlobalVariable(var_name, values);
            }
        };

        bind_globals("vertex_shader_global_variables", material->m_vertex_context);
        bind_globals("pixel_shader_global_variables", material->m_pixel_context);

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
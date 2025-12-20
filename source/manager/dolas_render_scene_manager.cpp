#include "base/dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "core/dolas_engine.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "tinyxml2.h"
#include <cstdlib>
namespace Dolas
{
    const RenderSceneID RenderSceneManager::RENDER_SCENE_ID_MAIN = STRING_ID(main_render_scene);

    RenderSceneManager::RenderSceneManager()
    {
    }

    RenderSceneManager::~RenderSceneManager()
    {
        Clear();
    }

    bool RenderSceneManager::Initialize()
    {
        return true;
    }

    bool RenderSceneManager::Clear()
    {
        for (auto it = m_render_scenes.begin(); it != m_render_scenes.end(); ++it)
        {
            RenderScene* scene = it->second;
            if (scene)
            {
                scene->Clear();
                DOLAS_DELETE(scene);
            }
        }
        m_render_scenes.clear();
        return true;
    }

    RenderScene* RenderSceneManager::GetRenderSceneByID(RenderSceneID id)
    {
        if (m_render_scenes.find(id) == m_render_scenes.end()) return nullptr;
        return m_render_scenes[id];
    }

    Bool RenderSceneManager::CreateRenderSceneByID(RenderSceneID id, const std::string& file_name)
    {
        DOLAS_RETURN_FALSE_IF_FALSE(m_render_scenes.find(id) == m_render_scenes.end());

		SceneRSD* scene_rsd = g_dolas_engine.m_asset_manager->GetSceneAsset(file_name);
		DOLAS_RETURN_FALSE_IF_NULL(scene_rsd);

        // 兼容现有渲染逻辑：把 SceneRSD（XML 片段字符串）映射回旧 SceneAsset 结构
        SceneAsset scene_asset_tmp{};
        for (const auto& entity_xml : scene_rsd->entities)
        {
            tinyxml2::XMLDocument d;
            if (d.Parse(entity_xml.c_str()) != tinyxml2::XML_SUCCESS)
            {
                LOG_ERROR("Invalid scene entity xml fragment!");
                continue;
            }

            const tinyxml2::XMLElement* e = d.RootElement();
            if (!e || std::string(e->Name()) != "entity")
            {
                LOG_ERROR("Invalid scene entity xml root!");
                continue;
            }

            const char* name = e->Attribute("name");
            const char* entity_file = e->Attribute("entity_file");
            if (!name || !entity_file)
            {
                LOG_ERROR("Scene entity xml missing name/entity_file attribute!");
                continue;
            }

            auto read_vec3 = [&](const char* key, Vector3& out) -> bool
            {
                const tinyxml2::XMLElement* el = e->FirstChildElement(key);
                if (!el) return false;
                double x = 0, y = 0, z = 0;
                if (el->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) return false;
                if (el->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) return false;
                if (el->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) return false;
                out = Vector3((Float)x, (Float)y, (Float)z);
                return true;
            };

            auto read_quat = [&](const char* key, Quaternion& out) -> bool
            {
                const tinyxml2::XMLElement* el = e->FirstChildElement(key);
                if (!el) return false;
                double w = 1, x = 0, y = 0, z = 0;
                if (el->QueryDoubleAttribute("w", &w) != tinyxml2::XML_SUCCESS) return false;
                if (el->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) return false;
                if (el->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) return false;
                if (el->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) return false;
                out = Quaternion((Float)w, (Float)x, (Float)y, (Float)z);
                return true;
            };

            SceneEntity scene_entity;
            scene_entity.name = name;
            scene_entity.entity_file = entity_file;
            if (!read_vec3("position", scene_entity.position) ||
                !read_quat("rotation", scene_entity.rotation) ||
                !read_vec3("scale", scene_entity.scale))
            {
                LOG_ERROR("Scene entity xml missing position/rotation/scale!");
                continue;
            }

            scene_asset_tmp.entities.push_back(scene_entity);
        }
        for (const auto& model_xml : scene_rsd->models)
        {
            tinyxml2::XMLDocument d;
            if (d.Parse(model_xml.c_str()) != tinyxml2::XML_SUCCESS)
                continue;
            const tinyxml2::XMLElement* m = d.RootElement();
            if (!m || std::string(m->Name()) != "model")
                continue;
            const char* n = m->Attribute("name");
            if (n) scene_asset_tmp.model_names.push_back(n);
        }

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        render_scene->BuildFromAsset(&scene_asset_tmp);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

		m_render_scenes[id] = render_scene;
        return true;
    }
} // namespace Dolas



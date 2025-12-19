#include "base/dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "core/dolas_engine.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
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

        // 兼容现有渲染逻辑：把 SceneRSD（json）映射回旧 SceneAsset 结构
        SceneAsset scene_asset_tmp{};
        for (const auto& entity_json : scene_rsd->entities)
        {
            if (entity_json.contains("name") && entity_json["name"].is_string() &&
                entity_json.contains("entity_file") && entity_json["entity_file"].is_string() &&
                entity_json.contains("position") && entity_json["position"].is_array() &&
                entity_json.contains("rotation") && entity_json["rotation"].is_array() &&
                entity_json.contains("scale") && entity_json["scale"].is_array())
            {
                SceneEntity scene_entity;
                scene_entity.name = entity_json["name"];
                scene_entity.entity_file = entity_json["entity_file"];
                scene_entity.position = Vector3(entity_json["position"][0], entity_json["position"][1], entity_json["position"][2]);
                scene_entity.rotation = Quaternion(entity_json["rotation"][0], entity_json["rotation"][1], entity_json["rotation"][2], entity_json["rotation"][3]);
                scene_entity.scale = Vector3(entity_json["scale"][0], entity_json["scale"][1], entity_json["scale"][2]);
                scene_asset_tmp.entities.push_back(scene_entity);
            }
            else
            {
                LOG_ERROR("Invalid scene entity format in SceneRSD json!");
            }
        }
        for (const auto& object_name : scene_rsd->models)
        {
            if (object_name.contains("name") && object_name["name"].is_string())
                scene_asset_tmp.model_names.push_back(object_name["name"]);
        }

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        render_scene->BuildFromAsset(&scene_asset_tmp);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

		m_render_scenes[id] = render_scene;
        return true;
    }
} // namespace Dolas



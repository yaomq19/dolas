#include "asset_types/scene_asset.h"
#include "dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "dolas_engine.h"
#include "dolas_asset_path.h"
#include "dolas_asset_manager.h"
#include "dolas_log_system_manager.h"
#include "manager/dolas_render_entity_manager.h"
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
        auto it = m_render_scenes.find(id);
        return (it != m_render_scenes.end()) ? it->second : nullptr;
    }

    Bool RenderSceneManager::CreateRenderSceneByID(RenderSceneID id, const AssetPath& asset_path)
    {
        DOLAS_RETURN_FALSE_IF_FALSE(m_render_scenes.find(id) == m_render_scenes.end());

		const auto load_result = g_dolas_engine.m_asset_manager->LoadAsset<SceneAssetDesc>(asset_path);
        if (!load_result)
        {
            LOG_ERROR(
                "Failed to load scene asset {0}: {1}",
                asset_path.GetCanonicalPath(),
                GetAssetLoadErrorName(load_result.GetError()));
            return false;
        }

		const SceneAssetDesc* scene_desc = load_result.GetAsset();

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

        for (const auto& item : scene_desc->entities)
        {
            // Required references have already been validated by AssetManager.
            const AssetPath& entity_asset_path = item.entity->GetPath();

            const Vector3 position = item.position;
            const Vector3 scale = item.scale;

            // Rotation stored as Vector4(x,y,z,w) in XML; Quaternion ctor is (w,x,y,z).
            const Vector4 rotv = item.rotation;
            const Quaternion rotation(rotv.w, rotv.x, rotv.y, rotv.z);

            RenderEntityID render_entity_id = g_dolas_engine.m_render_entity_manager->CreateRenderEntityFromFile(entity_asset_path, position, rotation, scale);
            if (render_entity_id != RENDER_ENTITY_ID_EMPTY)
            {
                render_scene->m_render_entities.push_back(render_entity_id);
            }
        }

		m_render_scenes[id] = render_scene;
        return true;
    }
} // namespace Dolas



#include "dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "dolas_engine.h"
#include "dolas_asset_manager.h"
#include "dolas_log_system_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "tinyxml2.h"
#include <cstdlib>
#include "rsd/scene.h"
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

		SceneRSD* scene_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<SceneRSD>(file_name);
		DOLAS_RETURN_FALSE_IF_NULL(scene_rsd);

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

        // 直接从 SceneRSD 构建 RenderScene（不再依赖旧 SceneAsset/SceneEntity）
        for (const auto& item : scene_rsd->entities)
        {
            const std::string& entity_file = item.entities;
            const Vector3 position = item.entity_positions;
            const Vector3 scale = item.entity_scales;

            // Rotation stored as Vector4(x,y,z,w) in XML; Quaternion ctor is (w,x,y,z).
            const Vector4 rotv = item.entity_rotations;
            const Quaternion rotation(rotv.w, rotv.x, rotv.y, rotv.z);

            RenderEntityID render_entity_id = g_dolas_engine.m_render_entity_manager->CreateRenderEntityFromFile(entity_file, position, rotation, scale);
            if (render_entity_id != RENDER_ENTITY_ID_EMPTY)
            {
                render_scene->m_render_entities.push_back(render_entity_id);
            }
        }

		m_render_scenes[id] = render_scene;
        return true;
    }
} // namespace Dolas



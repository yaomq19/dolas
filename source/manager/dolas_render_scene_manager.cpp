#include "base/dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "core/dolas_engine.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_render_entity_manager.h"
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

		SceneRSD* scene_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<SceneRSD>(PathUtils::GetSceneDir() + file_name);
		DOLAS_RETURN_FALSE_IF_NULL(scene_rsd);

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

        // 直接从 SceneRSD 构建 RenderScene（不再依赖旧 SceneAsset/SceneEntity）
        const std::size_t n = scene_rsd->entities.size();
        if (scene_rsd->entity_positions.size() != n ||
            scene_rsd->entity_rotations.size() != n ||
            scene_rsd->entity_scales.size() != n)
        {
            LOG_ERROR("SceneRSD arrays size mismatch: entities={0}, positions={1}, rotations={2}, scales={3}",
                (UInt)n,
                (UInt)scene_rsd->entity_positions.size(),
                (UInt)scene_rsd->entity_rotations.size(),
                (UInt)scene_rsd->entity_scales.size());
        }

        for (std::size_t i = 0; i < n; i++)
        {
            const std::string& entity_file = scene_rsd->entities[i];

            Vector3 position{};
            Vector3 scale{ 1.0f, 1.0f, 1.0f };
            Vector4 rotv{ 0.0f, 0.0f, 0.0f, 1.0f }; // (x,y,z,w)

            if (i < scene_rsd->entity_positions.size()) position = scene_rsd->entity_positions[i];
            if (i < scene_rsd->entity_scales.size()) scale = scene_rsd->entity_scales[i];
            if (i < scene_rsd->entity_rotations.size()) rotv = scene_rsd->entity_rotations[i];

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



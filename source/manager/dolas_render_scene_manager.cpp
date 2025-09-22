#include "base/dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"
#include "core/dolas_engine.h"
#include "manager/dolas_asset_manager.h"
namespace Dolas
{
    const RenderSceneID RenderSceneManager::RENDER_SCENE_ID_MAIN = STRING_ID(main_render_scene);

    RenderSceneManager::RenderSceneManager()
    {
    }

    RenderSceneManager::~RenderSceneManager()
    {
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

		SceneAsset* scene_asset = g_dolas_engine.m_asset_manager->GetSceneAsset(file_name);
		DOLAS_RETURN_FALSE_IF_NULL(scene_asset);

		RenderScene* render_scene = DOLAS_NEW(RenderScene);
        render_scene->BuildFromAsset(scene_asset);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);

		m_render_scenes[id] = render_scene;
        return true;
    }
} // namespace Dolas



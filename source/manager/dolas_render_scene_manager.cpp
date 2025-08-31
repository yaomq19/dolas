#include "base/dolas_base.h"
#include "manager/dolas_render_scene_manager.h"
#include "render/dolas_render_scene.h"

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

    Bool RenderSceneManager::CreateRenderSceneByID(RenderSceneID id)
    {
        DOLAS_RETURN_FALSE_IF_FALSE(m_render_scenes.find(id) == m_render_scenes.end());
		RenderScene* scene = DOLAS_NEW(RenderScene);
		DOLAS_RETURN_FALSE_IF_FALSE(scene->Initialize());
		m_render_scenes[id] = scene;
        return true;
    }
} // namespace Dolas



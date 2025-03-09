#include "Function/Render/RenderSceneManager/RenderSceneManager.h"

namespace Dolas
{
    RenderSceneManager::RenderSceneManager()
    {
        m_render_scene = new RenderScene();
    }

    RenderSceneManager::~RenderSceneManager()
    {
        delete m_render_scene;
    }

    bool RenderSceneManager::Initialize()
    {
        return m_render_scene->Initialize();
    }
}
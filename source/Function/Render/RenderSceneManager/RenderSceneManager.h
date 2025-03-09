#ifndef RENDERSCENEMANAGER_H
#define RENDERSCENEMANAGER_H

#include "Function/Render/RenderScene/RenderScene.h"
namespace Dolas
{
    class RenderSceneManager
    {
    public:
        RenderSceneManager();
        ~RenderSceneManager();
        bool Initialize();
        RenderScene* GetCurrentRenderScene() { return m_render_scene; }
        RenderScene* m_render_scene = nullptr;
    };
}

#endif // RENDERSCENEMANAGER_H
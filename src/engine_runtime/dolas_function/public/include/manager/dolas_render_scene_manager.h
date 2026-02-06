#ifndef DOLAS_RENDER_SCENE_MANAGER_H
#define DOLAS_RENDER_SCENE_MANAGER_H

#include <unordered_map>
#include <string>

#include "dolas_hash.h"

namespace Dolas
{
    class RenderScene;
    class RenderSceneManager
    {
    public:
        RenderSceneManager();
        ~RenderSceneManager();

        bool Initialize();
        bool Clear();

        RenderScene* GetRenderSceneByID(RenderSceneID id);
		Bool CreateRenderSceneByID(RenderSceneID id, const std::string& file_name);
    private:
        std::unordered_map<RenderSceneID, RenderScene*> m_render_scenes;
        const static RenderSceneID RENDER_SCENE_ID_MAIN;
    };// class RenderSceneManager
}// namespace Dolas

#endif // DOLAS_RENDER_SCENE_MANAGER_H



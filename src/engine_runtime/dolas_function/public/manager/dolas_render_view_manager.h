#ifndef DOLAS_RENDER_VIEW_MANAGER_H
#define DOLAS_RENDER_VIEW_MANAGER_H

#include <unordered_map>
#include <string>

#include "dolas_hash.h"

namespace Dolas
{
    class RenderView;
    class RenderViewManager
    {
    public:
        RenderViewManager();
        ~RenderViewManager();

        Bool Initialize();
        Bool Clear();

        RenderView* GetMainRenderView();
        RenderView* GetRenderView(RenderViewID id);

        Bool CreateRenderViewByID(RenderViewID id, RenderCameraID camera_id, RenderPipelineID pipeline_id, RenderResourceID resource_id, RenderSceneID scene_id);
    private:
        std::unordered_map<RenderViewID, RenderView*> m_render_views;
    };// class RenderViewManager
}// namespace Dolas

#endif // DOLAS_RENDER_VIEW_MANAGER_H



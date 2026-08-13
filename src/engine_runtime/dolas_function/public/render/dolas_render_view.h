#ifndef DOLAS_RENDER_VIEW_H
#define DOLAS_RENDER_VIEW_H

#include <string>
#include <memory>
#include "dolas_hash.h"
namespace Dolas
{
    class RenderCamera;
    class RenderPipeline;
    class RenderResource;
    class RenderScene;
    class DolasRHI;

    class RenderView
    {
        friend class RenderViewManager;
    public:
        RenderView();
        ~RenderView();

        bool Initialize();
        bool Clear();

        // 组件设置
        void SetRenderCameraID(RenderCameraID camera){ m_render_camera_id = camera; }
        void SetRenderPipelineID(RenderPipelineID pipeline) { m_render_pipeline_id = pipeline; }
        void SetRenderResourceID(RenderResourceID resource) { m_render_resource_id = resource; }
        void SetRenderSceneID(RenderSceneID scene) { m_render_scene_id = scene; }

        // 组件获取
        RenderCameraID GetRenderCameraID() const { return m_render_camera_id; }
        RenderPipelineID GetRenderPipelineID() const { return m_render_pipeline_id; }
        RenderResourceID GetRenderResourceID() const { return m_render_resource_id; }
        RenderSceneID GetRenderSceneID() const { return m_render_scene_id; }

        // 渲染执行
        void Render(DolasRHI* rhi);

    private:
        // 核心组件
        RenderCameraID m_render_camera_id;
        RenderPipelineID m_render_pipeline_id;
        RenderResourceID m_render_resource_id;
        RenderSceneID m_render_scene_id;

    }; // class RenderView
} // namespace Dolas

#endif // DOLAS_RENDER_VIEW_H 
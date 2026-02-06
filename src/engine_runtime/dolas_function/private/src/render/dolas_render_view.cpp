#include "render/dolas_render_view.h"
#include "render/dolas_render_camera.h"
#include "render/dolas_render_pipeline.h"
#include "render/dolas_render_resource.h"
#include "render/dolas_render_scene.h"
#include "render/dolas_rhi.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "dolas_engine.h"
#include <iostream>
#include <chrono>

namespace Dolas
{
    RenderView::RenderView()
    {

    }

    RenderView::~RenderView()
    {

    }

    bool RenderView::Initialize()
    {
        return true;
    }

    bool RenderView::Clear()
    {
        // 重置统计信息
        return true;
    }

    void RenderView::Render(DolasRHI* rhi)
    {
        std::wstring view_event_name = L"RenderView: ";
        UserAnnotationScope view_scope(rhi, view_event_name.c_str());

        RenderPipeline* render_pipeline = g_dolas_engine.m_render_pipeline_manager->GetRenderPipelineByID(m_render_pipeline_id);
        if (render_pipeline)
        {
            render_pipeline->Render(rhi);
        }

    }

} // namespace Dolas 
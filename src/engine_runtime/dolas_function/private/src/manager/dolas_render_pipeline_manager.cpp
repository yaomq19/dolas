#include "dolas_base.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "render/dolas_render_pipeline.h"
#include "manager/dolas_render_view_manager.h"
#include "dolas_engine.h"
#include "render/dolas_render_view.h"
namespace Dolas
{
    RenderPipelineManager::RenderPipelineManager()
    {

    }

    RenderPipelineManager::~RenderPipelineManager()
    {

    }

    bool RenderPipelineManager::Initialize()
    {
        return true;
    }

    bool RenderPipelineManager::Clear()
    {
        for (auto it = m_render_pipelines.begin(); it != m_render_pipelines.end(); ++it)
        {
            RenderPipeline* render_pipeline = it->second;
            if (render_pipeline)
            {
                render_pipeline->Clear();
                DOLAS_DELETE(render_pipeline);
            }
        }
        m_render_pipelines.clear();
        return true;
    }

    RenderPipeline* RenderPipelineManager::GetRenderPipelineByID(RenderPipelineID id)
    {
        return m_render_pipelines[id];
    }

    Bool RenderPipelineManager::CreateRenderPipelineByID(RenderPipelineID id)
    {
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_pipelines.find(id) == m_render_pipelines.end());
		RenderPipeline* render_pipeline = DOLAS_NEW(RenderPipeline);

        DOLAS_RETURN_FALSE_IF_FALSE(render_pipeline->Initialize());
        m_render_pipelines[id] = render_pipeline;
		return true;
    }

    void RenderPipelineManager::DisplayWorldCoordinateSystem()
    {
		RenderViewManager* render_view_manager = g_dolas_engine.m_render_view_manager;
        RenderView* main_render_view = render_view_manager->GetMainRenderView();
        RenderPipelineID render_pipeline_id = main_render_view->GetRenderPipelineID();
        RenderPipeline* render_pipeline = GetRenderPipelineByID(render_pipeline_id);
        render_pipeline->DisplayWorldCoordinateSystem();
    }

} // namespace Dolas

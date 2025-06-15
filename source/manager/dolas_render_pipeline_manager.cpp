#include "base/dolas_base.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "render/dolas_render_pipeline.h"

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
        m_render_pipelines["main"] = DOLAS_NEW(RenderPipeline);
        DOLAS_RETURN_FALSE_IF_FALSE(m_render_pipelines["main"]->Initialize());
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

    RenderPipeline* RenderPipelineManager::GetMainRenderPipeline()
    {
        return m_render_pipelines["main"];
    }

    RenderPipeline* RenderPipelineManager::GetRenderPipeline(const std::string& name)
    {
        return m_render_pipelines[name];
    }
} // namespace Dolas

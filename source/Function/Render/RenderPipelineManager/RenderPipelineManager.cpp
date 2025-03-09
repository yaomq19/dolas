#include "Function/Render/RenderPipelineManager/RenderPipelineManager.h"
#include "Function/Render/RenderPipeline/RenderPipeline.h"

namespace Dolas
{
    RenderPipelineManager::RenderPipelineManager()
    {
        m_render_pipeline = new RenderPipeline();
    }

    RenderPipelineManager::~RenderPipelineManager()
    {
        delete m_render_pipeline;
    }

    bool RenderPipelineManager::Initialize()
    {
        return m_render_pipeline->Initialize();
    }
}
#ifndef RENDER_PIPELINE_MANAGER_H
#define RENDER_PIPELINE_MANAGER_H

#include "Function/Render/RenderPipeline/RenderPipeline.h"
namespace Dolas
{
    class RenderPipelineManager
    {
    public:
        RenderPipelineManager();
        ~RenderPipelineManager();
        bool Initialize();
        RenderPipeline*         m_render_pipeline = nullptr;
        RenderPipeline* GetCurrentRenderPipeline() { return m_render_pipeline; }
    private:

    };
}

#endif // RENDER_PIPELINE_MANAGER_H
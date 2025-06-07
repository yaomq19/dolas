#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H

#include "dolas_rhi.h"
namespace Dolas
{
    class RenderPipeline
    {
    public:
        RenderPipeline();
        ~RenderPipeline();
        bool Initialize();
        void Render(DolasRHI* rhi);
    private:
        void ClearPass(DolasRHI* rhi);
        void GBufferPass(DolasRHI* rhi);
        void DeferredShadingPass(DolasRHI* rhi);
        void ForwardShadingPass(DolasRHI* rhi);
        void PostProcessPass(DolasRHI* rhi);
        void PresentPass(DolasRHI* rhi);
    };
}

#endif // DOLAS_RENDER_PIPELINE_H
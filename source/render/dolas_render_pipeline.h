#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H

namespace Dolas
{
    class DolasRHI;
    class RenderPipeline
    {
        friend class RenderPipelineManager;
    public:
        RenderPipeline();
        ~RenderPipeline();
        bool Initialize();
        bool Clear();
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
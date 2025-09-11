#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include <d3d11.h>
#include "common/dolas_hash.h"
#include "core/dolas_rhi.h"
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
        void SetRenderViewID(RenderViewID id);
    private:
        void ClearPass(DolasRHI* rhi);
        void GBufferPass(DolasRHI* rhi);
        void DeferredShadingPass(DolasRHI* rhi);
        void ForwardShadingPass(DolasRHI* rhi);
        void PostProcessPass(DolasRHI* rhi);
        void PresentPass(DolasRHI* rhi);

        class RenderResource* TryGetRenderResource() const;
        class RenderCamera* TryGetRenderCamera() const;
        ViewPort m_viewport;
        RenderViewID m_render_view_id;
    };// class RenderPipeline
}

#endif // DOLAS_RENDER_PIPELINE_H
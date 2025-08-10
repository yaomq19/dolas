#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include <d3d11.h>
#include "common/dolas_hash.h"

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

		D3D11_VIEWPORT m_viewport;
        RenderResourceID m_render_resource_id;
    };
}

#endif // DOLAS_RENDER_PIPELINE_H
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
    private:
        void ClearPass(DolasRHI* rhi);
        void GBufferPass(DolasRHI* rhi);
        void DeferredShadingPass(DolasRHI* rhi);
        void ForwardShadingPass(DolasRHI* rhi);
        void PostProcessPass(DolasRHI* rhi);
        void PresentPass(DolasRHI* rhi);

        ViewPort m_viewport;
        RasterizerState m_rasterizer_state;
        DepthStencilState m_depth_stencil_state;
        BlendState m_blend_state;
        RenderResourceID m_render_resource_id;
    };
}

#endif // DOLAS_RENDER_PIPELINE_H
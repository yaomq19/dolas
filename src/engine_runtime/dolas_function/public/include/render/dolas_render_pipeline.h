#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include <d3d11.h>
#include "dolas_hash.h"
#include "render/dolas_rhi.h"
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
        void DisplayWorldCoordinateSystem();
    private:
        void ClearPass(DolasRHI* rhi, class RenderView* render_view);
        void GBufferPass(DolasRHI* rhi, class RenderView* render_view);
        void DeferredShadingPass(DolasRHI* rhi, class RenderView* render_view);
        void ForwardShadingPass(DolasRHI* rhi);
        void SkyboxPass(DolasRHI* rhi, class RenderView* render_view);
        void DebugPass(DolasRHI* rhi, class RenderView* render_view);
        void ImGUIPass();
        void PostProcessPass(DolasRHI* rhi);
        void DisplayWorldCoordinate();
        void PresentPass(DolasRHI* rhi, class RenderView* render_view);

        class RenderScene* TryGetRenderScene(class RenderView* view = nullptr) const;
        class RenderResource* TryGetRenderResource(class RenderView* view = nullptr) const;
        class RenderCamera* TryGetRenderCamera(class RenderView* view = nullptr) const;
        class RenderView* TryGetRenderView() const;
        ViewPort m_viewport;
        RenderViewID m_render_view_id;

		Bool m_display_world_coordinate = false;
    };// class RenderPipeline
}

#endif // DOLAS_RENDER_PIPELINE_H
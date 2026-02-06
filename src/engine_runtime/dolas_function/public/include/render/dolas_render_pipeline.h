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
        void ClearPass(DolasRHI* rhi);
        void GBufferPass(DolasRHI* rhi);
        void DeferredShadingPass(DolasRHI* rhi);
        void ForwardShadingPass(DolasRHI* rhi);
        void SkyboxPass(DolasRHI* rhi);
        void DebugPass(DolasRHI* rhi);
        void ImGUIPass();
        void PostProcessPass(DolasRHI* rhi);
        void DisplayWorldCoordinate();
        void PresentPass(DolasRHI* rhi);

        class RenderScene* TryGetRenderScene() const;
        class RenderResource* TryGetRenderResource() const;
        class RenderCamera* TryGetRenderCamera() const;
        ViewPort m_viewport;
        RenderViewID m_render_view_id;

		Bool m_display_world_coordinate = false;
    };// class RenderPipeline
}

#endif // DOLAS_RENDER_PIPELINE_H
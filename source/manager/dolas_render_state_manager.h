#ifndef DOLAS_RENDER_STATE_MANAGER_H
#define DOLAS_RENDER_STATE_MANAGER_H

#include <unordered_map>
#include <d3d11.h>

#include "common/dolas_hash.h"
#include "core/dolas_rhi.h"

namespace Dolas
{
    enum class RasterizerStateType : UInt
    {
        SolidBackCull,
        SolidNoneCull,
        Wireframe,
        SolidFrontCull,
    };

    enum class DepthStencilStateType : UInt
    {
        DepthEnabled,
        DepthDisabled,
        DepthReadOnly,
    };

    enum class BlendStateType : UInt
    {
        Opaque,
        AlphaBlend,
        Additive,
    };

    class RenderStateManager
    {
    public:
        RenderStateManager();
        ~RenderStateManager();

        bool Initialize();
        bool Clear();

        RasterizerState* GetRasterizerState(RasterizerStateType type);
        DepthStencilState* GetDepthStencilState(DepthStencilStateType type);
        BlendState* GetBlendState(BlendStateType type);

    private:
        bool CreateRasterizerStates();
        bool CreateDepthStencilStates();
        bool CreateBlendStates();

        std::unordered_map<RasterizerStateType, RasterizerState*> m_rasterizer_states;
        std::unordered_map<DepthStencilStateType, DepthStencilState*> m_depth_stencil_states;
        std::unordered_map<BlendStateType, BlendState*> m_blend_states;
    };
}

#endif // DOLAS_RENDER_STATE_MANAGER_H



#include "manager/dolas_render_state_manager.h"

#include <iostream>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "base/dolas_base.h"

namespace Dolas
{
    RenderStateManager::RenderStateManager()
    {
    }

    RenderStateManager::~RenderStateManager()
    {
        Clear();
    }

    bool RenderStateManager::Initialize()
    {
        DOLAS_RETURN_FALSE_IF_FALSE(CreateRasterizerStates());
        DOLAS_RETURN_FALSE_IF_FALSE(CreateDepthStencilStates());
        DOLAS_RETURN_FALSE_IF_FALSE(CreateBlendStates());
        return true;
    }

    bool RenderStateManager::Clear()
    {
        for (auto& kv : m_rasterizer_states)
        {
            if (kv.second)
            {
                if (kv.second->m_d3d_rasterizer_state)
                {
                    kv.second->m_d3d_rasterizer_state->Release();
                    kv.second->m_d3d_rasterizer_state = nullptr;
                }
                delete kv.second;
            }
        }
        m_rasterizer_states.clear();

        for (auto& kv : m_depth_stencil_states)
        {
            if (kv.second)
            {
                if (kv.second->m_d3d_depth_stencil_state)
                {
                    kv.second->m_d3d_depth_stencil_state->Release();
                    kv.second->m_d3d_depth_stencil_state = nullptr;
                }
                delete kv.second;
            }
        }
        m_depth_stencil_states.clear();

        for (auto& kv : m_blend_states)
        {
            if (kv.second)
            {
                if (kv.second->m_d3d_blend_state)
                {
                    kv.second->m_d3d_blend_state->Release();
                    kv.second->m_d3d_blend_state = nullptr;
                }
                delete kv.second;
            }
        }
        m_blend_states.clear();

        return true;
    }

    RasterizerState* RenderStateManager::GetRasterizerState(RasterizerStateType type)
    {
        if (m_rasterizer_states.find(type) == m_rasterizer_states.end()) return nullptr;
        return m_rasterizer_states[type];
    }

    DepthStencilState* RenderStateManager::GetDepthStencilState(DepthStencilStateType type)
    {
        if (m_depth_stencil_states.find(type) == m_depth_stencil_states.end()) return nullptr;
        return m_depth_stencil_states[type];
    }

    BlendState* RenderStateManager::GetBlendState(BlendStateType type)
    {
        if (m_blend_states.find(type) == m_blend_states.end()) return nullptr;
        return m_blend_states[type];
    }

    bool RenderStateManager::CreateRasterizerStates()
    {
        DOLAS_RETURN_FALSE_IF_NULL(g_dolas_engine.m_rhi);
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;

        auto create_state = [&](const D3D11_RASTERIZER_DESC& desc, RasterizerStateType type)
        {
            RasterizerState* state = new RasterizerState();
            state->m_d3d_rasterizer_state = nullptr;
            HRESULT hr = device->CreateRasterizerState(&desc, &state->m_d3d_rasterizer_state);
            if (FAILED(hr)) { delete state; return false; }
            m_rasterizer_states[type] = state;
            return true;
        };

        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, RasterizerStateType::SolidBackCull));

        desc.CullMode = D3D11_CULL_NONE;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, RasterizerStateType::SolidNoneCull));

        desc.FillMode = D3D11_FILL_WIREFRAME; desc.CullMode = D3D11_CULL_BACK;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, RasterizerStateType::Wireframe));

        desc.FillMode = D3D11_FILL_SOLID; desc.CullMode = D3D11_CULL_FRONT;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, RasterizerStateType::SolidFrontCull));

        return true;
    }

    bool RenderStateManager::CreateDepthStencilStates()
    {
        DOLAS_RETURN_FALSE_IF_NULL(g_dolas_engine.m_rhi);
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;

        auto create_state = [&](const D3D11_DEPTH_STENCIL_DESC& desc, DepthStencilStateType type)
        {
            DepthStencilState* state = new DepthStencilState();
            state->m_d3d_depth_stencil_state = nullptr;
            HRESULT hr = device->CreateDepthStencilState(&desc, &state->m_d3d_depth_stencil_state);
            if (FAILED(hr)) { delete state; return false; }
            m_depth_stencil_states[type] = state;
            return true;
        };

        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = TRUE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS;
        desc.StencilEnable = FALSE;
        desc.StencilReadMask = 0xFF;
        desc.StencilWriteMask = 0xFF;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, DepthStencilStateType::DepthEnabled));

        desc.DepthEnable = FALSE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, DepthStencilStateType::DepthDisabled));

        desc.DepthEnable = TRUE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, DepthStencilStateType::DepthReadOnly));

        return true;
    }

    bool RenderStateManager::CreateBlendStates()
    {
        DOLAS_RETURN_FALSE_IF_NULL(g_dolas_engine.m_rhi);
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;

        auto create_state = [&](const D3D11_BLEND_DESC& desc, BlendStateType type)
        {
            BlendState* state = new BlendState();
            state->m_d3d_blend_state = nullptr;
            HRESULT hr = device->CreateBlendState(&desc, &state->m_d3d_blend_state);
            if (FAILED(hr)) { delete state; return false; }
            m_blend_states[type] = state;
            return true;
        };

        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;
        auto& rt = desc.RenderTarget[0];
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        // Opaque
        rt.BlendEnable = FALSE;
        rt.SrcBlend = D3D11_BLEND_ONE;
        rt.DestBlend = D3D11_BLEND_ZERO;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, BlendStateType::Opaque));

        // AlphaBlend
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, BlendStateType::AlphaBlend));

        // Additive
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_ONE;
        rt.DestBlend = D3D11_BLEND_ONE;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ONE;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        DOLAS_RETURN_FALSE_IF_FALSE(create_state(desc, BlendStateType::Additive));

        return true;
    }
}



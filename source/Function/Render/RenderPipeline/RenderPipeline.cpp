#include "Function/FrameWork/Dolas.h"
#include "Function/Render/RenderPipeline/RenderPipeline.h"

namespace Dolas
{
    RenderPipeline::RenderPipeline()
    {
    }

    RenderPipeline::~RenderPipeline()
    {
    }

    bool RenderPipeline::Initialize()
    {
        return true;
    }

    void RenderPipeline::Render()
    {
        // 获取当前渲染场景
        RenderScene* renderScene = g_dolas_context.m_render_scene_manager->GetCurrentRenderScene();
        for(const RenderEntity* renderEntity : renderScene->m_render_entities)
        {
            renderEntity->Render();
        }
        // 创建渲染目标视图
        ID3D11RenderTargetView* renderTargetView = nullptr;
        ID3D11Texture2D* backBuffer = g_dolas_context.m_texture_manager->GetBackBuffer();
        g_dolas_context.m_d3d_device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

        // 设置渲染目标
        g_dolas_context.m_d3d_deferred_context->OMSetRenderTargets(1, &renderTargetView, nullptr);

        // 设置视口
        D3D11_VIEWPORT vp;
        vp.Width = 800;
        vp.Height = 600;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_dolas_context.m_d3d_deferred_context->RSSetViewports(1, &vp);

        // 清除渲染目标
        float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
        g_dolas_context.m_d3d_deferred_context->ClearRenderTargetView(renderTargetView, clearColor);

        // 生成命令列表
        ID3D11CommandList* pCommandList = nullptr;
        g_dolas_context.m_d3d_deferred_context->FinishCommandList(FALSE, &pCommandList);

        // 执行命令列表
        g_dolas_context.m_d3d_immediate_context->ExecuteCommandList(pCommandList, FALSE);

        // 交换缓冲区
        g_dolas_context.m_d3d_swap_chain->Present(0, 0);
    }
}
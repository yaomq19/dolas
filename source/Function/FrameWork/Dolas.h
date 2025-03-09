#ifndef DOLAS_H
#define DOLAS_H
#include <windows.h>
#include <d3d11.h>

#include "Function/Render/RenderPipeline/RenderPipeline.h"
#include "Function/Render/RenderResourceManager/RenderResourceManager.h"
#include "Function/Render/TextureManager/TextureManager.h"
#include "Function/Render/RenderSceneManager/RenderSceneManager.h"
#include "Function/Render/RenderPipelineManager/RenderPipelineManager.h"
namespace Dolas
{
    class DolasContext
    {
    public:
        DolasContext();
        ~DolasContext();

        bool Initialize(HINSTANCE hInstance, int nCmdShow, unsigned int width, unsigned int height, LPWSTR title);
        void Run();
        void Shutdown();
        bool InitializeWindow(HINSTANCE hInstance, unsigned int width, unsigned int height, LPWSTR title);
        bool InitializeDirectX11();
        // HINSTANCE m_hInstance;
        HWND m_window;
        unsigned int m_window_width;
        unsigned int m_window_height;
        ID3D11Device*           m_d3d_device = nullptr;          // D3D11 设备
        ID3D11DeviceContext*    m_d3d_immediate_context = nullptr;// 设备上下文
        ID3D11DeviceContext*    m_d3d_deferred_context = nullptr;
        IDXGISwapChain*         m_d3d_swap_chain = nullptr;      // 交换链

        RenderResourceManager*  m_render_resource_manager = nullptr;
        TextureManager*         m_texture_manager = nullptr;
        RenderSceneManager*      m_render_scene_manager = nullptr;
        RenderPipelineManager*  m_render_pipeline_manager = nullptr;

    };
    extern DolasContext g_dolas_context;
}
#endif // DOLAS_H
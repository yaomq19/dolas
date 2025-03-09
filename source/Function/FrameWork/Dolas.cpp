#include "Function/FrameWork/Dolas.h"
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            // 用户点击关闭按钮时触发
            DestroyWindow(hwnd); // 销毁窗口并触发 WM_DESTROY
            return 0;

        case WM_DESTROY:
            // 窗口销毁时退出消息循环
            PostQuitMessage(0);  // 发送 WM_QUIT 到消息队列
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

namespace Dolas
{
    DolasContext g_dolas_context;
    DolasContext::DolasContext()
    {
        m_render_resource_manager = new RenderResourceManager();
        m_texture_manager = new TextureManager();
        m_render_pipeline_manager = new RenderPipelineManager();
        m_render_scene_manager = new RenderSceneManager();
    }

    DolasContext::~DolasContext()
    {
        delete m_render_resource_manager;
        delete m_texture_manager;
        delete m_render_pipeline_manager;
        delete m_render_scene_manager;
    }
    bool DolasContext::InitializeWindow(HINSTANCE hInstance, unsigned int width, unsigned int height, LPWSTR title)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"MyWindowClass";
        RegisterClass(&wc);

        m_window_width = width;
        m_window_height = height;

        m_window = CreateWindowEx(
            0, // 扩展样式，不需要
            L"MyWindowClass", // 窗口类名
            title, // 窗口标题
            WS_OVERLAPPEDWINDOW, // 窗口样式
            CW_USEDEFAULT, // 窗口的x坐标
            CW_USEDEFAULT, // 窗口的y坐标
            width, // 窗口的宽度
            height, // 窗口的高度
            NULL, // 父窗口句柄
            NULL, // 菜单句柄
            hInstance, // 应用程序实例句柄
            NULL); // 其他参数
        return true;
    }
    
    bool DolasContext::InitializeDirectX11()
    {
        HRESULT hr = S_OK;

        // --- 步骤 1: 创建设备与设备上下文 ---
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        createFlags |= D3D11_CREATE_DEVICE_DEBUG; // 启用调试层
#endif
        hr = D3D11CreateDevice(
            nullptr,                        // 默认显卡[6](@ref)
            D3D_DRIVER_TYPE_HARDWARE,       // 硬件加速驱动[8](@ref)
            nullptr,
            createFlags,
            featureLevels,
            1,
            D3D11_SDK_VERSION,
            &m_d3d_device,
            nullptr,
            &m_d3d_immediate_context
        );
        if (FAILED(hr)) return false;

        // --- 步骤 2: 创建交换链 ---
        DXGI_SWAP_CHAIN_DESC swapDesc = {};
        swapDesc.BufferCount = 2;                               // 双缓冲
        swapDesc.BufferDesc.Width = m_window_width;
        swapDesc.BufferDesc.Height = m_window_height;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// 32位颜色格式
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 作为渲染目标
        swapDesc.OutputWindow = m_window;                           // 目标窗口句柄
        swapDesc.SampleDesc.Count = 1;                          // 禁用多重采样
        swapDesc.Windowed = TRUE;                               // 窗口模式
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;         // 交换后丢弃旧帧

        IDXGIDevice* dxgiDevice = nullptr;
        m_d3d_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
        IDXGIAdapter* adapter = nullptr;
        dxgiDevice->GetAdapter(&adapter);
        IDXGIFactory* factory = nullptr;
        adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory);

        hr = factory->CreateSwapChain(m_d3d_device, &swapDesc, &m_d3d_swap_chain);
        adapter->Release();
        dxgiDevice->Release();
        factory->Release();
        if (FAILED(hr)) return false;

        // --- 步骤 2: 创建延迟上下文 ---
        hr = m_d3d_device->CreateDeferredContext(0, &m_d3d_deferred_context);
        if (FAILED(hr)) return false;


        return true;
    }


    bool DolasContext::Initialize(HINSTANCE hInstance, int nCmdShow, unsigned int width, unsigned int height, LPWSTR title)
    {
        bool success = InitializeWindow(hInstance, width, height, title);
        if(!success)
        {
            return false;
        }

        success = InitializeDirectX11();
        if(!success)
        {
            return false;
        }

        success = m_render_resource_manager->Initialize();;
        if(!success)
        {
            return false;
        }

        success = m_texture_manager->Initialize();
        if(!success)
        {
            return false;
        }

        success = m_render_pipeline_manager->Initialize();
        if(!success)
        {
            return false;
        }

        success = m_render_scene_manager->Initialize();
        if(!success)
        {
            return false;
        }
        return true;
    }

    void DolasContext::Run()
    {
        ShowWindow(m_window, SW_SHOW);
        MSG msg;
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            m_render_pipeline_manager->GetCurrentRenderPipeline()->Render();
        }
    }

    void DolasContext::Shutdown()
    {
        DestroyWindow(m_window);
        m_d3d_swap_chain->Release();
        m_d3d_immediate_context->Release();
        m_d3d_device->Release();
    }
}
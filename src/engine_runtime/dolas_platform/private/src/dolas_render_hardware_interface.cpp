#include <dxgi1_6.h>
#include <d3d12.h>
#if defined(_DEBUG) || defined(DEBUG)
#include <d3d12sdklayers.h>
#endif

#include "dolas_render_hardware_interface.h"
#include "dolas_log_system_manager.h"
namespace Dolas
{
    LRESULT CALLBACK MainWndProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Forward hwnd on because we can get messages (e.g., WM_CREATE)
        // before CreateWindow returns, and thus before m_hMainWnd is valid.
        // return g_dolas_engine.m_input_manager->MsgProc(hwnd, msg, wParam, lParam);
        
        // hack
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    RenderHardwareInterface::RenderHardwareInterface() = default;

    RenderHardwareInterface::~RenderHardwareInterface() = default;

    bool RenderHardwareInterface::Initialize()
    {
        if (!InitializeWindow(1270, 800))
        {
            return false;
        }
        
        if (!InitializeD3D12())
        {
            return false;
        }
        
        return true;
    }

    bool RenderHardwareInterface::Clear()
    {
        return true;
    }
    
    bool RenderHardwareInterface::InitializeWindow(LONG origin_width, LONG origin_height)
    {
        WNDCLASS wc;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWndProc2;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wc.lpszMenuName = 0;
        wc.lpszClassName = "D3DWndClassName_D3D12";

        if (!RegisterClass(&wc))
        {
            MessageBoxW(0, L"RegisterClass Failed.", 0, 0);
            return false;
        }

        // Compute window rectangle dimensions based on requested client area dimensions.
        // 注意：方案C - 这个窗口主要用于承载 SwapChain，可以最小化显示
        RECT R = { 0, 0, origin_width , origin_height };
        m_client_width = origin_width;
        m_client_height = origin_height;
        
        AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
        int real_width = R.right - R.left;
        int real_height = R.bottom - R.top;

        // 创建窗口（保持可见，因为 ImGui 主视口会使用它）
        m_window_hwnd = CreateWindowW(L"D3DWndClassName", L"Dolas Engine - Main Window",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, real_width, real_height, 0, 0, NULL, 0);
        if (!m_window_hwnd)
        {
            MessageBoxW(0, L"CreateWindow Failed.", 0, 0);
            return false;
        }

        // 显示窗口（ImGui 多视口需要主窗口可见）
        ShowWindow(m_window_hwnd, SW_SHOW);
        UpdateWindow(m_window_hwnd);

        // 启动时彻底禁用这个窗口上的 IME（直接输入，不经过中文输入法）
        // 这样键盘消息不会被输入法拦截，可直接用于游戏按键。
        ImmAssociateContext(m_window_hwnd, NULL);

        return true;
    }

    bool RenderHardwareInterface::InitializeD3D12()
    {
        HRESULT hr = S_OK;
        
        // 1. 开启调试层
#if defined(_DEBUG) || defined(DEBUG)
        ID3D12Debug* debug_controller = nullptr;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        if (SUCCEEDED(hr))
        {
            debug_controller->EnableDebugLayer();
        }
#endif
        
        // 2. 创建 DXGI Factory
        // IDXGIFactory7 最低支持的版本是 Windows 10, version 1809
        IDXGIFactory7* factory7;
        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory7));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create DXGI Factory! HRESULT: 0x{0:X}", hr);
            return false;
        }
        
        // 2. 选择最高性能的适配器（也就是物理显卡）
        // IDXGIAdapter4 最低支持的版本是 Windows 10, version 1803
        IDXGIAdapter4* dxgi_adapter4 = nullptr;
        for (UINT i = 0; SUCCEEDED(factory7->EnumAdapterByGpuPreference(
            i, 
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, // 核心：强制性能优先（通常选独显）
            IID_PPV_ARGS(&dxgi_adapter4))); ++i) 
        {
            // 检查是否是软件模拟，不是就 break 拿去用
            // DXGI_ADAPTER_DESC3 最低支持的版本是 Windows 10, version 1803
            DXGI_ADAPTER_DESC3 dxgi_adapter_desc3;
            dxgi_adapter4->GetDesc3(&dxgi_adapter_desc3);
            if (!(dxgi_adapter_desc3.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) break;
        }
        
        // 以上的代码都和显卡本身的版本无关，以下就有关了
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_12_2, // 尝试最高级 (Ultimate)
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_0  // 兜底
        };
        LOG_INFO("test");
        D3D_FEATURE_LEVEL max_supported_level = D3D_FEATURE_LEVEL_11_0; // 默认最低级
        for (auto level : levels) {
            // 仅探测，不创建实例
            if (SUCCEEDED(D3D12CreateDevice(dxgi_adapter4, level, _uuidof(::ID3D12Device), nullptr))) {
                max_supported_level = level; // 找到了显卡能支持的最高等级
                break;
            }
        }
        
        // 3. 创建 D3D12 设备
        hr = D3D12CreateDevice(dxgi_adapter4, max_supported_level, IID_PPV_ARGS(&m_device));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 Device! HRESULT: 0x{0:X}", hr);
            return false;
        }
        
        // 4. 创建命令队列
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.NodeMask = 0;
        
        hr = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 CommandQueue! HRESULT: 0x{0:X}", hr);
            return false;
        }
        
        // 5. 创建命令分配器
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 CommandAllocator! HRESULT: 0x{0:X}", hr);
            return false;
        }
        
        // 6. 创建命令列表
        hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator, nullptr, IID_PPV_ARGS(&m_command_list));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 CommandList! HRESULT: 0x{}", hr);
            return false;
        }
        
        // 7. 创建交换链
        IDXGISwapChain1* temp_swap_chain1 = nullptr;
        DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {};
        swap_chain_desc1.Width = m_client_width;                           // 窗口宽度
        swap_chain_desc1.Height = m_client_height;                         // 窗口高度
        swap_chain_desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // 像素格式
        swap_chain_desc1.Stereo = FALSE;
        swap_chain_desc1.SampleDesc.Count = 1;                    // D3D12 不在此处开启 MSAA
        swap_chain_desc1.SampleDesc.Quality = 0;
        swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 作为渲染目标
        swap_chain_desc1.BufferCount = 2;                         // 双缓冲（建议 2 或 3）
        swap_chain_desc1.Scaling = DXGI_SCALING_STRETCH;
        swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // D3D12 必须用 FLIP 模式
        swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swap_chain_desc1.Flags = 0;
        
        hr = factory7->CreateSwapChainForHwnd(
            m_command_queue,
            m_window_hwnd,
            &swap_chain_desc1,
            nullptr,
            nullptr,
            &temp_swap_chain1);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create SwapChain! HRESULT: 0x{0:X}", hr);
            return false;
        }
        
        // 升级到版本 4 以便管理 BackBuffer 索引
        hr = temp_swap_chain1->QueryInterface(IID_PPV_ARGS(&m_swap_chain4));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to query SwapChain4! HRESULT: 0x{0:X}", hr);
        }
        auto ret = temp_swap_chain1->Release(); // 释放临时接口
        
        
        return true;
    }
}

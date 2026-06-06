#include <dxgi1_6.h>
#include <d3d12.h>
#if defined(_DEBUG) || defined(DEBUG)
#include <d3d12sdklayers.h>
#endif

#include "dolas_render_hardware_interface.h"
#include "dolas_log_system_manager.h"
namespace Dolas
{
    namespace
    {
        constexpr wchar_t kD3D12WindowClassName[] = L"DolasD3D12WindowClass";
        RenderHardwareInterface::WindowMessageHandler g_window_message_handler = nullptr;

        template<typename T>
        void SafeRelease(T*& ptr)
        {
            if (ptr)
            {
                ptr->Release();
                ptr = nullptr;
            }
        }
    }

    LRESULT CALLBACK MainWndProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (g_window_message_handler)
        {
            return g_window_message_handler(hwnd, msg, wParam, lParam);
        }

        if (msg == WM_DESTROY)
        {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    RenderHardwareInterface::RenderHardwareInterface() = default;

    RenderHardwareInterface::~RenderHardwareInterface() = default;

    bool RenderHardwareInterface::Initialize()
    {
        if (!InitializeWindow(1920, 1080))
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
        WaitForGpu();

        if (m_fence_event)
        {
            CloseHandle(m_fence_event);
            m_fence_event = nullptr;
        }

        for (UINT i = 0; i < kFrameCount; ++i)
        {
            SafeRelease(m_render_targets[i]);
        }
        SafeRelease(m_fence);
        SafeRelease(m_srv_heap);
        SafeRelease(m_dsv_heap);
        SafeRelease(m_rtv_heap);
        SafeRelease(m_swap_chain4);
        SafeRelease(m_command_list);
        SafeRelease(m_command_allocator);
        SafeRelease(m_command_queue);
        SafeRelease(m_device);

        if (m_window_hwnd)
        {
            DestroyWindow(m_window_hwnd);
            m_window_hwnd = nullptr;
        }

        UnregisterClassW(kD3D12WindowClassName, GetModuleHandleW(nullptr));
        return true;
    }

    bool RenderHardwareInterface::BeginFrame(const float clear_color[4])
    {
        if (!m_command_allocator || !m_command_list || !m_render_targets[m_frame_index])
        {
            return false;
        }

        HRESULT hr = m_command_allocator->Reset();
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to reset D3D12 command allocator! HRESULT: 0x{0:X}", hr);
            return false;
        }

        hr = m_command_list->Reset(m_command_allocator, nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to reset D3D12 command list! HRESULT: 0x{0:X}", hr);
            return false;
        }

        ResetTransientSrvDescriptors();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_render_targets[m_frame_index];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_command_list->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = GetCurrentRtvHandle();
        m_command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
        m_command_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
        return true;
    }

    bool RenderHardwareInterface::EndFrame()
    {
        if (!m_command_list || !m_command_queue || !m_render_targets[m_frame_index])
        {
            return false;
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_render_targets[m_frame_index];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        m_command_list->ResourceBarrier(1, &barrier);

        HRESULT hr = m_command_list->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to close D3D12 command list! HRESULT: 0x{0:X}", hr);
            return false;
        }

        ID3D12CommandList* command_lists[] = { m_command_list };
        m_command_queue->ExecuteCommandLists(1, command_lists);
        return Present();
    }

    bool RenderHardwareInterface::Present()
    {
        if (!m_swap_chain4)
        {
            return false;
        }

        HRESULT hr = m_swap_chain4->Present(1, 0);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to present D3D12 swap chain! HRESULT: 0x{0:X}", hr);
            return false;
        }

        WaitForGpu();
        m_frame_index = m_swap_chain4->GetCurrentBackBufferIndex();
        return true;
    }

    bool RenderHardwareInterface::ExecuteImmediate(const std::function<bool(ID3D12GraphicsCommandList*)>& record_commands)
    {
        if (!m_command_allocator || !m_command_list || !m_command_queue || !record_commands)
        {
            return false;
        }

        WaitForGpu();

        HRESULT hr = m_command_allocator->Reset();
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to reset D3D12 immediate command allocator! HRESULT: 0x{0:X}", hr);
            return false;
        }

        hr = m_command_list->Reset(m_command_allocator, nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to reset D3D12 immediate command list! HRESULT: 0x{0:X}", hr);
            return false;
        }

        if (!record_commands(m_command_list))
        {
            m_command_list->Close();
            return false;
        }

        hr = m_command_list->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to close D3D12 immediate command list! HRESULT: 0x{0:X}", hr);
            return false;
        }

        ID3D12CommandList* command_lists[] = { m_command_list };
        m_command_queue->ExecuteCommandLists(1, command_lists);
        WaitForGpu();
        return true;
    }

    bool RenderHardwareInterface::AllocateRtvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle)
    {
        if (!m_rtv_heap || !out_cpu_handle || m_rtv_descriptor_next_index >= kRtvDescriptorCount)
        {
            LOG_ERROR("Failed to allocate D3D12 RTV descriptor.");
            return false;
        }

        const UINT descriptor_index = m_rtv_descriptor_next_index++;
        *out_cpu_handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
        out_cpu_handle->ptr += static_cast<SIZE_T>(descriptor_index) * m_rtv_descriptor_size;
        return true;
    }

    bool RenderHardwareInterface::AllocateDsvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle)
    {
        if (!m_dsv_heap || !out_cpu_handle || m_dsv_descriptor_next_index >= kDsvDescriptorCount)
        {
            LOG_ERROR("Failed to allocate D3D12 DSV descriptor.");
            return false;
        }

        const UINT descriptor_index = m_dsv_descriptor_next_index++;
        *out_cpu_handle = m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
        out_cpu_handle->ptr += static_cast<SIZE_T>(descriptor_index) * m_dsv_descriptor_size;
        return true;
    }

    bool RenderHardwareInterface::AllocateSrvDescriptor(
        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    {
        if (!m_srv_heap || !out_cpu_handle || !out_gpu_handle ||
            m_srv_descriptor_persistent_next_index >= kPersistentSrvDescriptorCount)
        {
            LOG_ERROR("Failed to allocate persistent D3D12 SRV descriptor.");
            return false;
        }

        const UINT descriptor_index = m_srv_descriptor_persistent_next_index++;
        *out_cpu_handle = m_srv_heap->GetCPUDescriptorHandleForHeapStart();
        *out_gpu_handle = m_srv_heap->GetGPUDescriptorHandleForHeapStart();
        out_cpu_handle->ptr += static_cast<SIZE_T>(descriptor_index) * m_srv_descriptor_size;
        out_gpu_handle->ptr += static_cast<UINT64>(descriptor_index) * m_srv_descriptor_size;
        return true;
    }

    bool RenderHardwareInterface::AllocateTransientSrvDescriptorTable(
        UINT descriptor_count,
        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    {
        if (!m_srv_heap || !out_cpu_handle || !out_gpu_handle || descriptor_count == 0 ||
            m_srv_descriptor_transient_next_index + descriptor_count > kSrvDescriptorCount)
        {
            LOG_ERROR("Failed to allocate transient D3D12 SRV descriptor table.");
            return false;
        }

        const UINT descriptor_index = m_srv_descriptor_transient_next_index;
        m_srv_descriptor_transient_next_index += descriptor_count;

        *out_cpu_handle = m_srv_heap->GetCPUDescriptorHandleForHeapStart();
        *out_gpu_handle = m_srv_heap->GetGPUDescriptorHandleForHeapStart();
        out_cpu_handle->ptr += static_cast<SIZE_T>(descriptor_index) * m_srv_descriptor_size;
        out_gpu_handle->ptr += static_cast<UINT64>(descriptor_index) * m_srv_descriptor_size;
        return true;
    }

    void RenderHardwareInterface::FreeSrvDescriptor(
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    {
        (void)cpu_handle;
        (void)gpu_handle;
    }

    void RenderHardwareInterface::ResetTransientSrvDescriptors()
    {
        m_srv_descriptor_transient_next_index = kPersistentSrvDescriptorCount;
    }

    void RenderHardwareInterface::SetWindowMessageHandler(WindowMessageHandler handler)
    {
        g_window_message_handler = handler;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderHardwareInterface::GetCurrentRtvHandle() const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
        if (!m_rtv_heap)
        {
            return handle;
        }

        handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(m_frame_index) * m_rtv_descriptor_size;
        return handle;
    }
    
    bool RenderHardwareInterface::InitializeWindow(LONG origin_width, LONG origin_height)
    {
        WNDCLASSW wc = {};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWndProc2;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = kD3D12WindowClassName;

        if (!RegisterClassW(&wc))
        {
            const DWORD last_error = GetLastError();
            if (last_error != ERROR_CLASS_ALREADY_EXISTS)
            {
                MessageBoxW(nullptr, L"RegisterClass Failed.", nullptr, 0);
                return false;
            }
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
        m_window_hwnd = CreateWindowW(kD3D12WindowClassName, L"Dolas Engine - Main Window",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, real_width, real_height, nullptr, nullptr, wc.hInstance, nullptr);
        if (!m_window_hwnd)
        {
            MessageBoxW(nullptr, L"CreateWindow Failed.", nullptr, 0);
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
        UINT factory_flags = 0;
        
        // 1. 开启调试层
#if defined(_DEBUG) || defined(DEBUG)
        ID3D12Debug* debug_controller = nullptr;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
        if (SUCCEEDED(hr))
        {
            debug_controller->EnableDebugLayer();
            factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
        }
        SafeRelease(debug_controller);
#endif
        
        // 2. 创建 DXGI Factory
        // IDXGIFactory7 最低支持的版本是 Windows 10, version 1809
        IDXGIFactory7* factory7 = nullptr;
        hr = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory7));
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
            SafeRelease(dxgi_adapter4);
        }
        if (!dxgi_adapter4)
        {
            LOG_ERROR("Failed to find a hardware DXGI adapter!");
            SafeRelease(factory7);
            return false;
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
            SafeRelease(dxgi_adapter4);
            SafeRelease(factory7);
            return false;
        }
        SafeRelease(dxgi_adapter4);
        
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
            SafeRelease(factory7);
            return false;
        }
        
        // 5. 创建命令分配器
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 CommandAllocator! HRESULT: 0x{0:X}", hr);
            SafeRelease(factory7);
            return false;
        }
        
        // 6. 创建命令列表
        hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator, nullptr, IID_PPV_ARGS(&m_command_list));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 CommandList! HRESULT: 0x{}", hr);
            SafeRelease(factory7);
            return false;
        }
        hr = m_command_list->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to close initial D3D12 CommandList! HRESULT: 0x{0:X}", hr);
            SafeRelease(factory7);
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
        swap_chain_desc1.BufferCount = kFrameCount;                         // 双缓冲（建议 2 或 3）
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
            SafeRelease(factory7);
            return false;
        }
        factory7->MakeWindowAssociation(m_window_hwnd, DXGI_MWA_NO_ALT_ENTER);
        
        // 升级到版本 4 以便管理 BackBuffer 索引
        hr = temp_swap_chain1->QueryInterface(IID_PPV_ARGS(&m_swap_chain4));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to query SwapChain4! HRESULT: 0x{0:X}", hr);
            SafeRelease(temp_swap_chain1);
            SafeRelease(factory7);
            return false;
        }
        SafeRelease(temp_swap_chain1);
        SafeRelease(factory7);
        m_frame_index = m_swap_chain4->GetCurrentBackBufferIndex();

        if (!CreateRenderTargetViews())
        {
            return false;
        }

        if (!CreateDepthStencilDescriptorHeap())
        {
            return false;
        }

        if (!CreateSrvDescriptorHeap())
        {
            return false;
        }

        hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 fence! HRESULT: 0x{0:X}", hr);
            return false;
        }
        m_fence_value = 1;

        m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fence_event)
        {
            LOG_ERROR("Failed to create D3D12 fence event!");
            return false;
        }
        
        
        return true;
    }

    bool RenderHardwareInterface::CreateSrvDescriptorHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
        srv_heap_desc.NumDescriptors = kSrvDescriptorCount;
        srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        HRESULT hr = m_device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&m_srv_heap));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 SRV descriptor heap! HRESULT: 0x{0:X}", hr);
            return false;
        }

        m_srv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_srv_descriptor_persistent_next_index = 0;
        ResetTransientSrvDescriptors();

        m_null_srv_cpu_handle = m_srv_heap->GetCPUDescriptorHandleForHeapStart();
        D3D12_SHADER_RESOURCE_VIEW_DESC null_srv_desc = {};
        null_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        null_srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        null_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        null_srv_desc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(nullptr, &null_srv_desc, m_null_srv_cpu_handle);
        m_srv_descriptor_persistent_next_index = 1;
        return true;
    }

    bool RenderHardwareInterface::CreateRenderTargetViews()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = kRtvDescriptorCount;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        HRESULT hr = m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtv_heap));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 RTV descriptor heap! HRESULT: 0x{0:X}", hr);
            return false;
        }

        m_rtv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_rtv_descriptor_next_index = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < kFrameCount; ++i)
        {
            hr = m_swap_chain4->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i]));
            if (FAILED(hr))
            {
                LOG_ERROR("Failed to get D3D12 swap-chain back buffer {0}! HRESULT: 0x{1:X}", i, hr);
                return false;
            }

            m_device->CreateRenderTargetView(m_render_targets[i], nullptr, rtv_handle);
            rtv_handle.ptr += m_rtv_descriptor_size;
            ++m_rtv_descriptor_next_index;
        }

        return true;
    }

    bool RenderHardwareInterface::CreateDepthStencilDescriptorHeap()
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
        dsv_heap_desc.NumDescriptors = kDsvDescriptorCount;
        dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        HRESULT hr = m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&m_dsv_heap));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create D3D12 DSV descriptor heap! HRESULT: 0x{0:X}", hr);
            return false;
        }

        m_dsv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        m_dsv_descriptor_next_index = 0;
        return true;
    }

    void RenderHardwareInterface::WaitForGpu()
    {
        if (!m_command_queue || !m_fence || !m_fence_event)
        {
            return;
        }

        const UINT64 fence_to_wait_for = m_fence_value;
        HRESULT hr = m_command_queue->Signal(m_fence, fence_to_wait_for);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to signal D3D12 fence! HRESULT: 0x{0:X}", hr);
            return;
        }
        ++m_fence_value;

        if (m_fence->GetCompletedValue() < fence_to_wait_for)
        {
            hr = m_fence->SetEventOnCompletion(fence_to_wait_for, m_fence_event);
            if (FAILED(hr))
            {
                LOG_ERROR("Failed to set D3D12 fence completion event! HRESULT: 0x{0:X}", hr);
                return;
            }
            WaitForSingleObject(m_fence_event, INFINITE);
        }
    }
}

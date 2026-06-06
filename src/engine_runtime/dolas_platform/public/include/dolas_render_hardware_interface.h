#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <functional>

namespace Dolas
{
    class RenderHardwareInterface
    {
    public:
        using WindowMessageHandler = LRESULT(*)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static constexpr UINT kFrameCount = 2;

        RenderHardwareInterface();
        ~RenderHardwareInterface();

        bool Initialize();
        bool Clear();
        bool BeginFrame(const float clear_color[4]);
        bool EndFrame();
        bool Present();
        bool ExecuteImmediate(const std::function<bool(ID3D12GraphicsCommandList*)>& record_commands);
        bool AllocateRtvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle);
        bool AllocateDsvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle);
        bool AllocateSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle);
        bool AllocateTransientSrvDescriptorTable(UINT descriptor_count, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle);
        void FreeSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle);
        void ResetTransientSrvDescriptors();
        void SetWindowMessageHandler(WindowMessageHandler handler);
        
        ID3D12Device* GetDevice() const { return m_device; }
        ID3D12CommandQueue* GetCommandQueue() const { return m_command_queue; }
        ID3D12GraphicsCommandList* GetCommandList() const { return m_command_list; }
        ID3D12DescriptorHeap* GetRtvHeap() const { return m_rtv_heap; }
        ID3D12DescriptorHeap* GetSrvHeap() const { return m_srv_heap; }
        UINT GetSrvDescriptorSize() const { return m_srv_descriptor_size; }
        IDXGISwapChain4* GetSwapChain() const { return m_swap_chain4; }
        HWND GetWindowHandle() const { return m_window_hwnd; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvHandle() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetNullSrvDescriptorCpuHandle() const { return m_null_srv_cpu_handle; }
        ID3D12Resource* GetCurrentBackBufferResource() const { return m_render_targets[m_frame_index]; }

    private:
        static constexpr UINT kRtvDescriptorCount = 256;
        static constexpr UINT kDsvDescriptorCount = 64;
        static constexpr UINT kSrvDescriptorCount = 1024;
        static constexpr UINT kPersistentSrvDescriptorCount = 512;

        bool InitializeWindow(LONG origin_width, LONG origin_height);
        bool InitializeD3D12();
        bool CreateRenderTargetViews();
        bool CreateDepthStencilDescriptorHeap();
        bool CreateSrvDescriptorHeap();
        void WaitForGpu();
        
        ID3D12Device* m_device {nullptr};
        ID3D12CommandQueue* m_command_queue {nullptr};
        ID3D12CommandAllocator* m_command_allocator {nullptr};
        ID3D12GraphicsCommandList* m_command_list {nullptr};
        IDXGISwapChain4* m_swap_chain4 {nullptr};
        ID3D12DescriptorHeap* m_rtv_heap {nullptr};
        ID3D12DescriptorHeap* m_dsv_heap {nullptr};
        ID3D12DescriptorHeap* m_srv_heap {nullptr};
        ID3D12Resource* m_render_targets[kFrameCount] {nullptr, nullptr};
        ID3D12Fence* m_fence {nullptr};
        HANDLE m_fence_event {nullptr};
        UINT m_rtv_descriptor_size {0};
        UINT m_dsv_descriptor_size {0};
        UINT m_srv_descriptor_size {0};
        UINT m_rtv_descriptor_next_index {0};
        UINT m_dsv_descriptor_next_index {0};
        UINT m_srv_descriptor_persistent_next_index {0};
        UINT m_srv_descriptor_transient_next_index {0};
        D3D12_CPU_DESCRIPTOR_HANDLE m_null_srv_cpu_handle {};
        UINT m_frame_index {0};
        UINT64 m_fence_value {0};
        HWND m_window_hwnd {nullptr};
        LONG m_client_width {1280};
        LONG m_client_height {720};
    };
}

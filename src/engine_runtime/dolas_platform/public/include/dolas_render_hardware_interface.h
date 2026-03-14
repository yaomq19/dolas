#pragma once

namespace Dolas
{
    class RenderHardwareInterface
    {
    public:
        RenderHardwareInterface();
        ~RenderHardwareInterface();

        bool Initialize();
        bool Clear();
        
        class ID3D12Device* GetDevice() const { return m_device; }
        class ID3D12CommandQueue* GetCommandQueue() const { return m_command_queue; }

    private:
        bool InitializeWindow(LONG origin_width, LONG origin_height);
        bool InitializeD3D12();
        
        class ID3D12Device* m_device {nullptr};
        class ID3D12CommandQueue* m_command_queue {nullptr};
        class ID3D12CommandAllocator* m_command_allocator {nullptr};
        class ID3D12CommandList* m_command_list {nullptr};
        class IDXGISwapChain4* m_swap_chain4 {nullptr};
        HWND m_window_hwnd;
        LONG m_client_width {1280};
        LONG m_client_height {720};
    };
}
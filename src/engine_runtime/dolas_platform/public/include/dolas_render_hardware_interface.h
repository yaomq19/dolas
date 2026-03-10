#pragma once
#include <d3d12.h>

namespace Dolas
{
    class RenderHardwareInterface
    {
    public:
        RenderHardwareInterface();
        ~RenderHardwareInterface();

        ID3D12Device* GetDevice() const { return m_device; }
        ID3D12CommandQueue* GetCommandQueue() const { return m_command_queue; }

    private:
        ID3D12Device* m_device;
        ID3D12CommandQueue* m_command_queue;
    };
}
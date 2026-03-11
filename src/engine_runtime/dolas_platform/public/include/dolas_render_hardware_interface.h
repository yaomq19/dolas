#pragma once

namespace Dolas
{
    class RenderHardwareInterface
    {
    public:
        RenderHardwareInterface();
        ~RenderHardwareInterface();

        class ID3D12Device* GetDevice() const { return m_device; }
        class ID3D12CommandQueue* GetCommandQueue() const { return m_command_queue; }

    private:
        class ID3D12Device* m_device {nullptr};
        class ID3D12CommandQueue* m_command_queue {nullptr};
    };
}
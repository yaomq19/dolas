#include "dolas_render_hardware_interface.h"
#include "dolas_log_system_manager.h"
namespace Dolas
{
    RenderHardwareInterface::RenderHardwareInterface()
    {
        // // 初始化 D3D12 设备
        // HRESULT hr = D3D12CreateDevice(
        //     nullptr,                    // 默认适配器
        //     D3D_FEATURE_LEVEL_11_0,     // 最低功能级别
        //     IID_PPV_ARGS(&m_device)    // 输出设备指针
        // );
        // if (FAILED(hr))
        // {
        //     // 设备创建失败，记录错误日志
        //     LOG_ERROR("Failed to create D3D12 device! HRESULT: 0x{0:X}", hr);
        //     m_device = nullptr;
        //     m_command_queue = nullptr;
        //     return;
        // }
    }

    RenderHardwareInterface::~RenderHardwareInterface() = default;
}

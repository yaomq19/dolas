#include <dxgi1_6.h>
#include <d3d12.h>
#if defined(_DEBUG) || defined(DEBUG)
#include <d3d12sdklayers.h>
#endif

#include "dolas_render_hardware_interface.h"
#include "dolas_log_system_manager.h"
namespace Dolas
{
    RenderHardwareInterface::RenderHardwareInterface()
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
            return;
        }
        
        // 2. 按“高性能”偏好枚举
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
            return;
        }
    }

    RenderHardwareInterface::~RenderHardwareInterface() = default;
}

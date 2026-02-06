#include "render/dxgi_helper.h"
#include "manager/dolas_log_system_manager.h"
#include "dolas_string_util.h"
#include "render/dolas_dx_trace.h"
namespace Dolas
{
    // Select the best performing adapter
    IDXGIAdapter* DXGIHelper::SelectBestAdapter()
    {
        IDXGIFactory* pFactory = nullptr;
        HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory));
        
        IDXGIAdapter* bestAdapter = nullptr;
		DXGI_ADAPTER_DESC best_adpater_desc;
        int bestScore = -1;
        
        // Loop through all adapters
        LOG_INFO("Auto choose a graphics processor adapter:");
        IDXGIAdapter* currentAdapter = nullptr;
        for (int i = 0; pFactory->EnumAdapters(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_ADAPTER_DESC desc;
            HR(currentAdapter->GetDesc(&desc));
			int current_score = GetAdapterPerformanceScore(desc);
            if (current_score > bestScore)
            {
				bestScore = current_score;
                if (bestAdapter) {
                    bestAdapter->Release();
                }
                bestAdapter = currentAdapter;
				bestAdapter->AddRef();
				best_adpater_desc = desc;
            }
            
            currentAdapter->Release();
        }
        
        pFactory->Release();
        
        if (bestAdapter) {
            LOG_INFO("Selected Best Adapter Name:{0} with score: {1}", StringUtil::WStringToString(best_adpater_desc.Description).c_str(), bestScore);
        } else {
            LOG_ERROR("No suitable adapter found!");
        }
        
        return bestAdapter;
    }

    // Print detailed adapter information
    void DXGIHelper::PrintAdapterInfo(IDXGIAdapter* pAdapter)
    {
        if (!pAdapter) {
            LOG_ERROR("Adapter is null!");
            return;
        }
        DXGI_ADAPTER_DESC desc;
        HR(pAdapter->GetDesc(&desc));

        // Output basic information
        LOG_INFO("======Adapter Info======");
        LOG_INFO("Description: {0}", StringUtil::WStringToString(desc.Description));
        // Output vendor and device ID (hexadecimal)
        LOG_INFO("Vendor ID: {0}", desc.VendorId);
        LOG_INFO("Device ID: {0}", desc.DeviceId);
        LOG_INFO("Subsystem ID: {0}", desc.SubSysId);
        LOG_INFO("Revision: {0}", desc.Revision);
        // Output memory information (convert to MB)
        const float MB = 1024.0f * 1024.0f;
        LOG_INFO("Dedicated Video Memory: {0} MB", desc.DedicatedVideoMemory / MB);
        LOG_INFO("Dedicated System Memory: {0} MB", desc.DedicatedSystemMemory / MB);
        LOG_INFO("Shared System Memory: {0} MB", desc.SharedSystemMemory / MB);
        // Output LUID
        LOG_INFO("Adapter LUID: {0}:{1}", desc.AdapterLuid.HighPart, desc.AdapterLuid.LowPart);
        LOG_INFO("========================");
    }

    // Calculate adapter performance score
    int DXGIHelper::GetAdapterPerformanceScore(const DXGI_ADAPTER_DESC& desc)
    {
        int score = 0;
        // Video memory is the most important factor (1 point per MB)
        score += static_cast<int>(desc.DedicatedVideoMemory / (1024 * 1024));
        // Bonus points for discrete graphics vendors
        if (desc.VendorId == 0x10DE) {  // NVIDIA
            score += 1000;
        } else if (desc.VendorId == 0x1002) {  // AMD
            score += 800;
        } else if (desc.VendorId == 0x8086) {  // Intel
            score += 500;
        }
        // Penalty for Microsoft Basic Render Driver
        if (desc.VendorId == 0x1414) {  // Microsoft
            score -= 10000;
        }
        return score;
    }
} // namespace Dolas
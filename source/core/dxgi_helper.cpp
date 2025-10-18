#include "dxgi_helper.h"
#include "manager/dolas_log_system_manager.h"
#include "base/dolas_string_util.h"
namespace Dolas
{
    // Select the best performing adapter
    IDXGIAdapter* DXGIHelper::SelectBestAdapter()
    {
        IDXGIFactory* pFactory = nullptr;
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
        if (FAILED(hr)) {
            std::cout << "Failed to create DXGI Factory!" << std::endl;
            return nullptr;
        }
        
        IDXGIAdapter* bestAdapter = nullptr;
        IDXGIAdapter* currentAdapter = nullptr;
        int bestScore = -1;
        int bestIndex = -1;
        
        // Loop through all adapters
        LOG_INFO("Auto choose a graphics processor adapter:");
        for (int i = 0; pFactory->EnumAdapters(i, &currentAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_ADAPTER_DESC desc;
            hr = currentAdapter->GetDesc(&desc);
            
            if (SUCCEEDED(hr)) {
                int score = GetAdapterPerformanceScore(desc);
				std::wstring descs(desc.Description);
                LOG_INFO("Graphics Processor Adapter index: {0} score : {1}, description : {2}", i, score, StringUtil::WStringToString(descs));
                if (score > bestScore) {
                    bestScore = score;
                    bestIndex = i;
                    
                    // Release previous best adapter
                    if (bestAdapter) {
                        bestAdapter->Release();
                    }
                    
                    // Store current adapter as best (add reference)
                    bestAdapter = currentAdapter;
                    bestAdapter->AddRef();
                }
            }
            
            currentAdapter->Release();
        }
        
        pFactory->Release();
        
        if (bestAdapter) {
            std::cout << "\n*** Selected Best Adapter " << bestIndex 
                    << " with score: " << bestScore << " ***" << std::endl;
        } else {
            std::cout << "No suitable adapter found!" << std::endl;
        }
        
        return bestAdapter;
    }

    // Print detailed adapter information
    void DXGIHelper::PrintAdapterInfo(IDXGIAdapter* pAdapter)
    {
        if (!pAdapter) {
            std::cout << "Adapter is null!" << std::endl;
            return;
        }
        DXGI_ADAPTER_DESC desc;
        HRESULT hr = pAdapter->GetDesc(&desc);
        
        if (FAILED(hr)) {
            std::cout << "Failed to get adapter description!" << std::endl;
            return;
        }
        // Output basic information
        std::cout << "======Adapter Info======" << std::endl;
        std::wcout << L"Description: " << desc.Description << std::endl;
        // Output vendor and device ID (hexadecimal)
        std::cout << "Vendor ID: 0x" << std::hex << desc.VendorId << std::dec << std::endl;
        std::cout << "Device ID: 0x" << std::hex << desc.DeviceId << std::dec << std::endl;
        std::cout << "Subsystem ID: 0x" << std::hex << desc.SubSysId << std::dec << std::endl;
        std::cout << "Revision: 0x" << std::hex << desc.Revision << std::dec << std::endl;
        // Output memory information (convert to MB)
        const float MB = 1024.0f * 1024.0f;
        std::cout << "Dedicated Video Memory: " << desc.DedicatedVideoMemory / MB << " MB" << std::endl;
        std::cout << "Dedicated System Memory: " << desc.DedicatedSystemMemory / MB << " MB" << std::endl;
        std::cout << "Shared System Memory: " << desc.SharedSystemMemory / MB << " MB" << std::endl;
        // Output LUID
        std::cout << "Adapter LUID: " << desc.AdapterLuid.HighPart 
                << ":" << desc.AdapterLuid.LowPart << std::endl;
        std::cout << "========================" << std::endl;
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
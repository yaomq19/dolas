#ifndef DXGI_HELPER_H
#define DXGI_HELPER_H

#include <dxgi.h>
#include <iostream>
namespace Dolas
{
    class DXGIHelper
    {
    public:
        static IDXGIAdapter* SelectBestAdapter();
        static void PrintAdapterInfo(IDXGIAdapter* pAdapter);
    private:
        static int GetAdapterPerformanceScore(const DXGI_ADAPTER_DESC& desc);
    };
} // namespace Dolas


#endif // DXGI_HELPER_H 
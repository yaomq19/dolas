
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "dolas_rhi.h"

namespace Dolas
{
RHI::RHI()
{

}

RHI::~RHI()
{

}

bool RHI::initMainWindow()
{
    if (!glfwInit())
    {
        return false;
    }

    m_window = glfwCreateWindow(800, 600, "Dolas Window", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    return true;
}

bool RHI::initialize()
{
    initMainWindow();
    // Create a Direct3D swap chain.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 800;
    sd.BufferDesc.Height = 600;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = glfwGetWin32Window(m_window);
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
        0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        0,                          // Set debug and Direct2D compatibility flags.
        nullptr,                    // List of feature levels this app can support.
        0,                          // Number of elements in the feature level array.
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
        &sd,                        // Swap chain description.
        &m_swap_chain,              // Returns the swap chain created.
        &m_device,                  // Returns the Direct3D device created.
        &featureLevel,              // Returns feature level of device created.
        &m_device_context           // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        return false;
    }

    // Create a render target view.
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_render_target_view);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool RHI::initResources()
{
    // Set the viewport.
    m_viewport.Width = 800.0f;
    m_viewport.Height = 600.0f;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    return true;
}
}
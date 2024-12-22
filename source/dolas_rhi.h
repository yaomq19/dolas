#ifndef DOLAS_RHI_H
#define DOLAS_RHI_H
#include <GLFW/glfw3.h>
#include "dolas_common.h"
#include "d3d11.h"
namespace Dolas
{
class RHI
{
public:
    RHI();
    ~RHI();
    bool initialize();

    ID3D11DeviceContext* getDeviceContext() { return m_device_context; }
    ID3D11Device* getDevice() { return m_device; }
    IDXGISwapChain* getSwapChain() { return m_swap_chain; }
    ID3D11RenderTargetView* getRenderTargetView() { return m_render_target_view; }

    D3D11_VIEWPORT getViewPort() { return m_viewport; }
private:
    bool initMainWindow();
    bool initResources();
    GLFWwindow* m_window;
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_device_context;
    IDXGISwapChain* m_swap_chain;
    ID3D11RenderTargetView* m_render_target_view;

    D3D11_VIEWPORT m_viewport;
};
}
#endif
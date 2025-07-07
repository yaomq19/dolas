#include "dolas_rhi.h"
#include "dolas_engine.h"
#include "dxgi_helper.h"

namespace Dolas
{
    LRESULT CALLBACK
    MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Forward hwnd on because we can get messages (e.g., WM_CREATE)
        // before CreateWindow returns, and thus before m_hMainWnd is valid.
        return g_dolas_engine.m_rhi->MsgProc(hwnd, msg, wParam, lParam);
    }

	DolasRHI::DolasRHI()
		: m_d3d_device(nullptr)
		, m_d3d_immediate_context(nullptr)
		, m_swap_chain(nullptr)
		, m_back_buffer(nullptr)
		, m_render_target_view(nullptr)
		, m_window_handle(nullptr)
		, m_client_width(1920)
		, m_client_height(1080)
	{
		// 初始化D3D设备和上下文
	}

	DolasRHI::~DolasRHI()
	{
		// 释放D3D设备和上下文
		if (m_back_buffer) m_back_buffer->Release();
		if (m_d3d_immediate_context) m_d3d_immediate_context->Release();
		if (m_d3d_device) m_d3d_device->Release();
		if (m_swap_chain) m_swap_chain->Release();
	}

	bool DolasRHI::Initialize()
	{
		if (!InitializeWindow()) return false;
		if (!InitializeD3D()) return false;
		return true;
	}

	void DolasRHI::Present()
	{
		m_swap_chain->Present(0, 0);
	}

    LRESULT DolasRHI::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE)
                {
                    PostQuitMessage(0);
                }
                return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
	bool DolasRHI::InitializeWindow()
	{
		WNDCLASS wc;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wc.lpszMenuName = 0;
        wc.lpszClassName = L"D3DWndClassName";

        if (!RegisterClass(&wc))
        {
            MessageBox(0, L"RegisterClass Failed.", 0, 0);
            return false;
        }

        // Compute window rectangle dimensions based on requested client area dimensions.
        RECT R = { 0, 0, m_client_width , m_client_height };
        AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
        int width = R.right - R.left;
        int height = R.bottom - R.top;

        m_window_handle = CreateWindowW(L"D3DWndClassName", L"Dolas Engine",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, NULL, 0);
        if (!m_window_handle)
        {
            MessageBoxW(0, L"CreateWindow Failed.", 0, 0);
            return false;
        }

        ShowWindow(m_window_handle, SW_SHOW);
        UpdateWindow(m_window_handle);

        return true;
	}

	bool DolasRHI::InitializeD3D()
	{
        IDXGIAdapter* best_adapter = DXGIHelper::SelectBestAdapter();
		if (!best_adapter) {
			std::cout << "Failed to select best adapter!" << std::endl;
			return false;
		}

		DXGI_ADAPTER_DESC adapter_desc;
		best_adapter->GetDesc(&adapter_desc);
		DXGIHelper::PrintAdapterInfo(best_adapter);

		// 设置交换链描述
		DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
		swap_chain_desc.BufferCount = 1;
		swap_chain_desc.BufferDesc.Width = m_client_width;
		swap_chain_desc.BufferDesc.Height = m_client_height;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.OutputWindow = m_window_handle;  // 使用创建的窗口句柄
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
		swap_chain_desc.Windowed = TRUE;

		// 设置特性级别
		D3D_FEATURE_LEVEL feature_levels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};
		D3D_FEATURE_LEVEL feature_level;
#ifdef _DEBUG
		UINT d3d_device_create_flags = 0;
		d3d_device_create_flags |= D3D11_CREATE_DEVICE_DEBUG;  // 开发时启用调试信息
#endif
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			best_adapter,                    // pAdapter
			D3D_DRIVER_TYPE_UNKNOWN,         // DriverType (使用指定适配器时必须是UNKNOWN)
			NULL,                            // Software
			d3d_device_create_flags,         // Flags
			feature_levels,                  // pFeatureLevels
			ARRAYSIZE(feature_levels),       // FeatureLevels
			D3D11_SDK_VERSION,               // SDKVersion
			&swap_chain_desc,                // pSwapChainDesc
			&m_swap_chain,                   // ppSwapChain
			&m_d3d_device,                   // ppDevice
			&feature_level,                  // pFeatureLevel
			&m_d3d_immediate_context         // ppImmediateContext
		);

        hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_back_buffer));
        if (FAILED(hr)) {
            std::cout << "Failed to get back buffer! HRESULT: 0x" 
                      << std::hex << hr << std::dec << std::endl;
            return false;
        }

        hr = m_d3d_device->CreateRenderTargetView(m_back_buffer, nullptr, &m_render_target_view);
        if (FAILED(hr)) {
            std::cout << "Failed to create render target view! HRESULT: 0x" 
                      << std::hex << hr << std::dec << std::endl;
            return false;
        }

		// 释放适配器
		best_adapter->Release();

		if (FAILED(hr)) {
			std::cout << "Failed to create D3D11 device and swap chain! HRESULT: 0x" 
					  << std::hex << hr << std::dec << std::endl;
			return false;
		}

		std::cout << "Successfully created D3D11 device and swap chain!" << std::endl;
		std::cout << "Feature Level: " << std::hex << feature_level << std::dec << std::endl;
		return true;
	}
} // namespace Dolas

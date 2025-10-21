#include "dolas_rhi.h"
#include "dolas_engine.h"
#include "dxgi_helper.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_input_manager.h"
#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For D3D11 debug interfaces
#endif

#include <iostream>
#include "render/dolas_render_camera.h"
#include "manager/dolas_log_system_manager.h"

namespace Dolas
{

    LRESULT CALLBACK
    MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Forward hwnd on because we can get messages (e.g., WM_CREATE)
        // before CreateWindow returns, and thus before m_hMainWnd is valid.
        return g_dolas_engine.m_input_manager->MsgProc(hwnd, msg, wParam, lParam);
    }

	RenderTargetView::RenderTargetView()
		: m_d3d_render_target_view(nullptr)
	{

	}

    RenderTargetView::RenderTargetView(TextureID texture_id)
    {
        m_d3d_render_target_view = nullptr;

        TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
        DOLAS_RETURN_IF_NULL(texture_manager);

        Texture* texture = texture_manager->GetTextureByTextureID(texture_id);
        DOLAS_RETURN_IF_NULL(texture);

        ID3D11Device* device = g_dolas_engine.m_rhi ? g_dolas_engine.m_rhi->m_d3d_device : nullptr;
        DOLAS_RETURN_IF_NULL(device);

        D3D11_TEXTURE2D_DESC texture_desc = {};
        texture->GetD3DTexture2D()->GetDesc(&texture_desc);

        D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
        rtv_desc.Format = texture_desc.Format;

        if (texture_desc.SampleDesc.Count > 1)
        {
            if (texture_desc.ArraySize > 1)
            {
                rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
                rtv_desc.Texture2DMSArray.ArraySize = texture_desc.ArraySize;
            }
            else
            {
                rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
        }
        else
        {
            if (texture_desc.ArraySize > 1)
            {
                rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtv_desc.Texture2DArray.MipSlice = 0;
                rtv_desc.Texture2DArray.FirstArraySlice = 0;
                rtv_desc.Texture2DArray.ArraySize = texture_desc.ArraySize;
            }
            else
            {
                rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtv_desc.Texture2D.MipSlice = 0;
            }
        }

        HRESULT hr = device->CreateRenderTargetView(texture->GetD3DTexture2D(), &rtv_desc, &m_d3d_render_target_view);
        if (FAILED(hr))
        {
            std::cout << "Failed to create render target view!" << std::endl;
            return;
        }
    }

    RenderTargetView::~RenderTargetView()
    {
        Release();
    }

    void RenderTargetView::Release()
    {
        if (m_d3d_render_target_view)
        {
            m_d3d_render_target_view->Release();
            m_d3d_render_target_view = nullptr;
        }
    }

    RenderTargetView::RenderTargetView(const RenderTargetView& other)
    {
        m_d3d_render_target_view = other.m_d3d_render_target_view;
        if (m_d3d_render_target_view) m_d3d_render_target_view->AddRef();
    }

    RenderTargetView& RenderTargetView::operator=(const RenderTargetView& other)
    {
        if (this == &other) return *this;
        Release();
        m_d3d_render_target_view = other.m_d3d_render_target_view;
        if (m_d3d_render_target_view) m_d3d_render_target_view->AddRef();
        return *this;
    }

    RenderTargetView::RenderTargetView(RenderTargetView&& other) noexcept
    {
        m_d3d_render_target_view = other.m_d3d_render_target_view;
        other.m_d3d_render_target_view = nullptr;
    }

    RenderTargetView& RenderTargetView::operator=(RenderTargetView&& other) noexcept
    {
        if (this == &other) return *this;
        Release();
        m_d3d_render_target_view = other.m_d3d_render_target_view;
        other.m_d3d_render_target_view = nullptr;
        return *this;
    }
    
	DepthStencilView::DepthStencilView() : m_d3d_depth_stencil_view(nullptr)
	{

	}

	DepthStencilView::DepthStencilView(TextureID texture_id)
	{
		m_d3d_depth_stencil_view = nullptr;

		TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
		DOLAS_RETURN_IF_NULL(texture_manager);

		Texture* texture = texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_IF_NULL(texture);

		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture->GetD3DTexture2D()->GetDesc(&texture_desc);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
		dsv_desc.Format = ConvertToDSVFormat(texture_desc.Format);
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv_desc.Texture2D.MipSlice = 0;

		HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateDepthStencilView(texture->GetD3DTexture2D(), &dsv_desc, &m_d3d_depth_stencil_view);
		if (FAILED(hr))
		{
			std::cout << "Failed to create depth stencil view!" << std::endl;
			return;
		}
	}

	DepthStencilView::~DepthStencilView()
	{
        Release();
	}	

    void DepthStencilView::Release()
    {
        if (m_d3d_depth_stencil_view)
        {
            m_d3d_depth_stencil_view->Release();
            m_d3d_depth_stencil_view = nullptr;
        }
    }

	DXGI_FORMAT DepthStencilView::ConvertToDSVFormat(DXGI_FORMAT origin_format)
	{
		DXGI_FORMAT ret_format = origin_format;
		switch (origin_format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			ret_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
        case DXGI_FORMAT_R32_TYPELESS:
            ret_format = DXGI_FORMAT_D32_FLOAT;
            break;
		default:
			break;
		}
		return ret_format;
	}

    DepthStencilView::DepthStencilView(const DepthStencilView& other)
    {
        m_d3d_depth_stencil_view = other.m_d3d_depth_stencil_view;
        if (m_d3d_depth_stencil_view) m_d3d_depth_stencil_view->AddRef();
    }

    DepthStencilView& DepthStencilView::operator=(const DepthStencilView& other)
    {
        if (this == &other) return *this;
        Release();
        m_d3d_depth_stencil_view = other.m_d3d_depth_stencil_view;
        if (m_d3d_depth_stencil_view) m_d3d_depth_stencil_view->AddRef();
        return *this;
    }

    DepthStencilView::DepthStencilView(DepthStencilView&& other) noexcept
    {
        m_d3d_depth_stencil_view = other.m_d3d_depth_stencil_view;
        other.m_d3d_depth_stencil_view = nullptr;
    }

    DepthStencilView& DepthStencilView::operator=(DepthStencilView&& other) noexcept
    {
        if (this == &other) return *this;
        Release();
        m_d3d_depth_stencil_view = other.m_d3d_depth_stencil_view;
        other.m_d3d_depth_stencil_view = nullptr;
        return *this;
    }

	DolasRHI::DolasRHI()
		: m_d3d_device(nullptr)
		, m_d3d_immediate_context(nullptr)
		, m_swap_chain(nullptr)
		, m_swap_chain_back_texture(nullptr)
		, m_back_buffer_render_target_view(nullptr)
		, m_client_width(DEFAULT_CLIENT_WIDTH)
		, m_client_height(DEFAULT_CLIENT_HEIGHT)
		, m_d3d_user_annotation(nullptr)
	{
		// 初始化D3D设备和上下文
	}

	DolasRHI::~DolasRHI()
	{
		// 释放D3D设备和上下文
		if (m_swap_chain_back_texture) m_swap_chain_back_texture->Release();
		if (m_d3d_user_annotation) m_d3d_user_annotation->Release();
		if (m_d3d_immediate_context) m_d3d_immediate_context->Release();
		if (m_d3d_device) m_d3d_device->Release();
		if (m_swap_chain) m_swap_chain->Release();
		if (m_back_buffer_render_target_view) m_back_buffer_render_target_view->Release();
	}

	bool DolasRHI::Initialize()
	{
		if (!InitializeWindow()) return false;
		if (!InitializeD3D()) return false;

		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(PerViewConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		// 新建常量缓冲区，不使用初始数据
		HRESULT hr = m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_view_parameters_buffer);
		if (FAILED(hr))
		{
			std::cout << "Failed to create per view constant buffer!" << std::endl;
			return false;
		}

		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(PerFrameConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		// 新建常量缓冲区，不使用初始数据
		hr = m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_frame_parameters_buffer);
		if (FAILED(hr))
		{
			std::cout << "Failed to create per frame constant buffer!" << std::endl;
			return false;
		}
		return true;
	}

	void DolasRHI::Clear()
	{
		// 释放常量缓冲区
		if (m_d3d_per_frame_parameters_buffer) { m_d3d_per_frame_parameters_buffer->Release(); m_d3d_per_frame_parameters_buffer = nullptr; }
		if (m_d3d_per_view_parameters_buffer) { m_d3d_per_view_parameters_buffer->Release(); m_d3d_per_view_parameters_buffer = nullptr; }
		
		if (m_swap_chain_back_texture) { m_swap_chain_back_texture->Release(); m_swap_chain_back_texture = nullptr; }
		if (m_back_buffer_render_target_view) { m_back_buffer_render_target_view->Release(); m_back_buffer_render_target_view = nullptr; }
		if (m_d3d_user_annotation) { m_d3d_user_annotation->Release(); m_d3d_user_annotation = nullptr; }
		if (m_d3d_immediate_context) { m_d3d_immediate_context->Release(); m_d3d_immediate_context = nullptr; }
		if (m_d3d_device) { m_d3d_device->Release(); m_d3d_device = nullptr; }
		if (m_swap_chain) { m_swap_chain->Release(); m_swap_chain = nullptr; }
	}

	void DolasRHI::Present()
	{
		m_swap_chain->Present(0, 0);
	}
    
	void DolasRHI::SetRenderTargetView(const std::vector<RenderTargetView>& d3d11_render_target_view, const DepthStencilView& d3d11_depth_stencil_view)
	{
		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
        if (d3d11_render_target_view.size() == 0) {
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, d3d11_depth_stencil_view.GetD3DDepthStencilView());
            return;
        }

        ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
            d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i].GetD3DRenderTargetView() : nullptr;
		}
        m_d3d_immediate_context->OMSetRenderTargets(d3d11_render_target_view.size(), d3d11_render_target_view_array, d3d11_depth_stencil_view.GetD3DDepthStencilView());
	}

    void DolasRHI::SetRenderTargetView(const std::vector<RenderTargetView>& d3d11_render_target_view)
    {
		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
		if (d3d11_render_target_view.size() == 0) {
			return;
		}

		ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
			d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i].GetD3DRenderTargetView() : nullptr;
		}
		m_d3d_immediate_context->OMSetRenderTargets(d3d11_render_target_view.size(), d3d11_render_target_view_array, nullptr);
    }

    void DolasRHI::SetViewPort(const ViewPort& viewport)
	{
		m_d3d_immediate_context->RSSetViewports(1, &viewport.m_d3d_viewport);
	}

    void DolasRHI::SetRasterizerState(const RasterizerState& rasterizer_state)
    {
        m_d3d_immediate_context->RSSetState(rasterizer_state.m_d3d_rasterizer_state);
    }

    void DolasRHI::SetDepthStencilState(const DepthStencilState& depth_stencil_state)
    {
        m_d3d_immediate_context->OMSetDepthStencilState(depth_stencil_state.m_d3d_depth_stencil_state, 0);
    }

    void DolasRHI::SetBlendState(const BlendState& blend_state)
    {
        m_d3d_immediate_context->OMSetBlendState(blend_state.m_d3d_blend_state, nullptr, 0xFFFFFFFF);
    }

	void DolasRHI::SetVertexShader()
	{

	}

	void DolasRHI::SetPixelShader()
	{

	}

	void DolasRHI::VSSetConstantBuffers()
	{
		m_d3d_immediate_context->VSSetConstantBuffers(0, 1, &m_d3d_per_view_parameters_buffer);
		m_d3d_immediate_context->VSSetConstantBuffers(1, 1, &m_d3d_per_frame_parameters_buffer);
	}

	void DolasRHI::PSSetConstantBuffers()
	{
		m_d3d_immediate_context->PSSetConstantBuffers(0, 1, &m_d3d_per_view_parameters_buffer);
		m_d3d_immediate_context->PSSetConstantBuffers(1, 1, &m_d3d_per_frame_parameters_buffer);
	}

	ID3D11ShaderResourceView* DolasRHI::CreateShaderResourceView(ID3D11Resource* resource)
	{
		ID3D11ShaderResourceView* shader_resource_view = nullptr;
		HRESULT hr = m_d3d_device->CreateShaderResourceView(resource, nullptr, &shader_resource_view);
		if (FAILED(hr))
		{
			std::cout << "Failed to create shader resource view!" << std::endl;
			return nullptr;
		}
		return shader_resource_view;
	}
	
	void DolasRHI::UpdatePerFrameParameters()
	{
		PerFrameConstantBuffer per_frame_constant_buffer;
		per_frame_constant_buffer.light_direction_intensity = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);
		per_frame_constant_buffer.light_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = m_d3d_immediate_context->Map(m_d3d_per_frame_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		if (FAILED(hr))
		{
			std::cout << "Failed to map per view constant buffer!" << std::endl;
			return;
		}
		memcpy_s(mappedData.pData, sizeof(per_frame_constant_buffer), &per_frame_constant_buffer, sizeof(per_frame_constant_buffer));
		m_d3d_immediate_context->Unmap(m_d3d_per_frame_parameters_buffer, 0);
	}

	void DolasRHI::UpdatePerViewParameters(RenderCamera* render_camera)
	{
		PerViewConstantBuffer per_view_constant_buffer;
		per_view_constant_buffer.view = render_camera->GetViewMatrix();
		per_view_constant_buffer.proj = render_camera->GetProjectionMatrix();
		per_view_constant_buffer.camera_position = Vector4(render_camera->GetPosition(), 1.0f);

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = m_d3d_immediate_context->Map(m_d3d_per_view_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		if (FAILED(hr))
		{
			std::cout << "Failed to map per view constant buffer!" << std::endl;
			return;
		}
		memcpy_s(mappedData.pData, sizeof(per_view_constant_buffer), &per_view_constant_buffer, sizeof(per_view_constant_buffer));
		m_d3d_immediate_context->Unmap(m_d3d_per_view_parameters_buffer, 0);
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
        wc.lpszClassName = "D3DWndClassName";

        if (!RegisterClass(&wc))
        {
            MessageBoxW(0, L"RegisterClass Failed.", 0, 0);
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

		// 设置设备创建标志
		UINT d3d_device_create_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		// 在调试模式下启用D3D11 debug layer
		d3d_device_create_flags |= D3D11_CREATE_DEVICE_DEBUG;
		LOG_INFO("D3D11 Debug Layer enabled!");
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

		// 释放适配器
		best_adapter->Release();

		if (FAILED(hr)) {
			LOG_ERROR("Failed to create D3D11 device and swap chain! HRESULT: {0}", hr);
			return false;
		}

		// 查询 GPU 事件注解接口（RenderDoc/PIX 标记）
		hr = m_d3d_immediate_context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_d3d_user_annotation));
		if (SUCCEEDED(hr) && m_d3d_user_annotation)
		{
			LOG_INFO("ID3DUserDefinedAnnotation acquired.");
		}
		else
		{
			m_d3d_user_annotation = nullptr;
		}

        hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_swap_chain_back_texture));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to get back buffer! HRESULT: {0}", hr);
            return false;
        }

        hr = m_d3d_device->CreateRenderTargetView(m_swap_chain_back_texture, nullptr, &m_back_buffer_render_target_view);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create render target view! HRESULT: {0}", hr);
            return false;
        }

#if defined(DEBUG) || defined(_DEBUG)
		// 启用D3D11 debug layer的额外配置
		ID3D11Debug* d3d_debug = nullptr;
		hr = m_d3d_device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3d_debug));
		if (SUCCEEDED(hr)) {
			ID3D11InfoQueue* d3d_info_queue = nullptr;
			hr = d3d_debug->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&d3d_info_queue));
			if (SUCCEEDED(hr)) {
				// 设置断点在错误和警告时
				d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
				// 可选：在警告时也断点（可能会比较频繁）
				// d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
				
				LOG_INFO("D3D11 Debug Info Queue configured!");
				d3d_info_queue->Release();
			}
			d3d_debug->Release();
		}
#endif

		LOG_INFO("Successfully created D3D11 device and swap chain!");
		LOG_INFO("Feature Level: {0:#x}", static_cast<int>(feature_level));
		return true;
	}

	void DolasRHI::BeginEvent(const wchar_t* name)
	{
		if (m_d3d_user_annotation)
		{
			m_d3d_user_annotation->BeginEvent(name);
		}
	}

	void DolasRHI::EndEvent()
	{
		if (m_d3d_user_annotation)
		{
			m_d3d_user_annotation->EndEvent();
		}
	}

	void DolasRHI::SetMarker(const wchar_t* name)
	{
		if (m_d3d_user_annotation)
		{
			m_d3d_user_annotation->SetMarker(name);
		}
	}
} // namespace Dolas

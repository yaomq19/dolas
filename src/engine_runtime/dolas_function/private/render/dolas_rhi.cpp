#include "render/dolas_rhi.h"
#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include "dolas_engine.h"
#include "dolas_render_hardware_interface.h"
#include "render/dxgi_helper.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_imgui_manager.h"
#include "render/dolas_dx_trace.h"
#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For D3D11 debug interfaces
#endif

#include <iostream>
#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>
#include "render/dolas_render_camera.h"
#include "dolas_log_system_manager.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_shader.h"
#include "render/dolas_texture.h"
namespace Dolas
{
	struct DolasRHI::D3D11StateCache
	{
		D3D11_RASTERIZER_DESC rasterizer_state_create_desc[RasterizerStateType_Count] = {};
		std::pair<D3D11_DEPTH_STENCIL_DESC, UInt> depth_stencil_state_create_desc[DepthStencilStateType_Count] = {};
		D3D11_BLEND_DESC blend_state_create_desc[BlendStateType_Count] = {};

		D3D11_PRIMITIVE_TOPOLOGY primitive_topology[PrimitiveTopology_Count] = {};
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_element_descs[InputLayoutType_Count];

		D3D12_RASTERIZER_DESC d3d12_rasterizer_state_create_desc[RasterizerStateType_Count] = {};
		std::pair<D3D12_DEPTH_STENCIL_DESC, UInt> d3d12_depth_stencil_state_create_desc[DepthStencilStateType_Count] = {};
		D3D12_BLEND_DESC d3d12_blend_state_create_desc[BlendStateType_Count] = {};
		D3D12_PRIMITIVE_TOPOLOGY d3d12_primitive_topology[PrimitiveTopology_Count] = {};
		D3D12_PRIMITIVE_TOPOLOGY_TYPE d3d12_primitive_topology_type[PrimitiveTopology_Count] = {};
		std::vector<D3D12_INPUT_ELEMENT_DESC> d3d12_input_element_descs[InputLayoutType_Count];
	};

	namespace
	{
		constexpr UINT kD3D12SrvTableSize = 16;
		constexpr UINT kRootPerViewCBV = 0;
		constexpr UINT kRootPerFrameCBV = 1;
		constexpr UINT kRootPerObjectCBV = 2;
		constexpr UINT kRootVSGlobalCBV = 3;
		constexpr UINT kRootPSGlobalCBV = 4;
		constexpr UINT kRootVSSrvTable = 5;
		constexpr UINT kRootPSSrvTable = 6;

		template<typename T>
		void SafeRelease(T*& ptr)
		{
			if (ptr)
			{
				ptr->Release();
				ptr = nullptr;
			}
		}

		std::size_t HashCombine(std::size_t seed, std::size_t value)
		{
			return seed ^ (value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2));
		}

		UINT AlignTo256(UINT value)
		{
			return (value + 255u) & ~255u;
		}

		bool CreateD3D12UploadBuffer(ID3D12Device* device, UINT size, const void* initial_data, ID3D12Resource** resource)
		{
			if (!device || !resource || size == 0)
			{
				return false;
			}

			D3D12_HEAP_PROPERTIES heap_properties = {};
			heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
			heap_properties.CreationNodeMask = 1;
			heap_properties.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC resource_desc = {};
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Width = AlignTo256(size);
			resource_desc.Height = 1;
			resource_desc.DepthOrArraySize = 1;
			resource_desc.MipLevels = 1;
			resource_desc.SampleDesc.Count = 1;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			HRESULT hr = device->CreateCommittedResource(
				&heap_properties,
				D3D12_HEAP_FLAG_NONE,
				&resource_desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(resource));
			if (FAILED(hr))
			{
				LOG_ERROR("Failed to create D3D12 upload buffer, HRESULT: 0x{0:X}", hr);
				return false;
			}

			if (initial_data)
			{
				D3D12_RANGE read_range = { 0, 0 };
				void* mapped_data = nullptr;
				hr = (*resource)->Map(0, &read_range, &mapped_data);
				if (FAILED(hr))
				{
					LOG_ERROR("Failed to map D3D12 upload buffer, HRESULT: 0x{0:X}", hr);
					SafeRelease(*resource);
					return false;
				}
				memcpy(mapped_data, initial_data, size);
				D3D12_RANGE written_range = { 0, size };
				(*resource)->Unmap(0, &written_range);
			}

			return true;
		}
	}

	RenderTargetView::RenderTargetView() : m_d3d_render_target_view(nullptr)
	{

	}

    RenderTargetView::~RenderTargetView()
    {
		if (m_d3d_render_target_view)
		{
			m_d3d_render_target_view->Release();
			m_d3d_render_target_view = nullptr;
		}
    }
    
	DepthStencilView::DepthStencilView() : m_d3d_depth_stencil_view(nullptr)
	{

	}

	DepthStencilView::~DepthStencilView()
	{
		if (m_d3d_depth_stencil_view)
		{
			m_d3d_depth_stencil_view->Release();
			m_d3d_depth_stencil_view = nullptr;
		}
	}	

	ViewPort::ViewPort(Float top_left_x, Float top_left_y, Float width, Float height, Float min_depth, Float max_depth)
		: m_top_left_x(top_left_x)
		, m_top_left_y(top_left_y)
		, m_width(width)
		, m_height(height)
		, m_min_depth(min_depth)
		, m_max_depth(max_depth)
	{
	}
	ViewPort::~ViewPort()
	{
	}

	RasterizerState::RasterizerState() : m_d3d_rasterizer_state(nullptr)
	{
	}

	RasterizerState::~RasterizerState()
	{
		if (m_d3d_rasterizer_state)
		{
			m_d3d_rasterizer_state->Release();
			m_d3d_rasterizer_state = nullptr;
		}
	}

	DepthStencilState::DepthStencilState() : m_d3d_depth_stencil_state(nullptr)
	{
	}

	DepthStencilState::~DepthStencilState()
	{
		if (m_d3d_depth_stencil_state)
		{
			m_d3d_depth_stencil_state->Release();
			m_d3d_depth_stencil_state = nullptr;
		}
	}

	BlendState::BlendState() : m_d3d_blend_state(nullptr)
	{
	}

	BlendState::~BlendState()
	{
		if (m_d3d_blend_state)
		{
			m_d3d_blend_state->Release();
			m_d3d_blend_state = nullptr;
		}
	}

	InputLayout::InputLayout() : m_d3d_input_layout(nullptr)
	{

	}

	InputLayout::~InputLayout()
	{
		if (m_d3d_input_layout)
		{
			m_d3d_input_layout->Release();
			m_d3d_input_layout = nullptr;
		}
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
		, m_d3d11_state_cache(std::make_unique<D3D11StateCache>())
	{
		// 初始化D3D设备和上下文
	}

	DolasRHI::~DolasRHI()
	{
		Clear();
	}

	bool DolasRHI::Initialize()
	{
		if (!InitializeD3D12CompatibilityResources()) return false;

		if (!InitializeD3D11CompatibilityDevice())
		{
			LOG_WARN("DolasRHI: D3D11 compatibility device unavailable; continuing with DX12-only runtime resources.");
		}

		if (m_d3d_device)
		{
			D3D11_BUFFER_DESC cbd;
			ZeroMemory(&cbd, sizeof(cbd));
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.ByteWidth = sizeof(PerViewConstantBuffer);
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			// 新建常量缓冲区，不使用初始数据
			HR(m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_view_parameters_buffer));

			ZeroMemory(&cbd, sizeof(cbd));
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.ByteWidth = sizeof(PerFrameConstantBuffer);
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			// 新建常量缓冲区，不使用初始数据
			HR(m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_frame_parameters_buffer));

			// Per object
			ZeroMemory(&cbd, sizeof(cbd));
			cbd.Usage = D3D11_USAGE_DYNAMIC;
			cbd.ByteWidth = sizeof(PerObjectConstantBuffer);
			cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			// 新建常量缓冲区，不使用初始数据
			HR(m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_object_parameters_buffer));
		}

		return true;
	}

	void DolasRHI::Clear()
	{
		// 释放常量缓冲区
		if (m_d3d_per_frame_parameters_buffer) { m_d3d_per_frame_parameters_buffer->Release(); m_d3d_per_frame_parameters_buffer = nullptr; }
		if (m_d3d_per_view_parameters_buffer) { m_d3d_per_view_parameters_buffer->Release(); m_d3d_per_view_parameters_buffer = nullptr; }
		if (m_d3d_per_object_parameters_buffer) { m_d3d_per_object_parameters_buffer->Release(); m_d3d_per_object_parameters_buffer = nullptr; }

		if (m_swap_chain_back_texture) { m_swap_chain_back_texture->Release(); m_swap_chain_back_texture = nullptr; }
		if (m_d3d_user_annotation) { m_d3d_user_annotation->Release(); m_d3d_user_annotation = nullptr; }
		if (m_d3d_immediate_context) { m_d3d_immediate_context->Release(); m_d3d_immediate_context = nullptr; }
		if (m_d3d_device) { m_d3d_device->Release(); m_d3d_device = nullptr; }
		if (m_swap_chain) { m_swap_chain->Release(); m_swap_chain = nullptr; }

		for (auto& pso_pair : m_d3d12_pipeline_state_cache)
		{
			SafeRelease(pso_pair.second);
		}
		m_d3d12_pipeline_state_cache.clear();
		SafeRelease(m_d3d12_root_signature);
		SafeRelease(m_d3d12_per_frame_parameters_buffer);
		SafeRelease(m_d3d12_per_view_parameters_buffer);
		SafeRelease(m_d3d12_per_object_parameters_buffer);
		SafeRelease(m_d3d12_dummy_constant_buffer);
	}

	bool DolasRHI::BeginFrame(const float clear_color[4])
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		if (!rhi || !rhi->BeginFrame(clear_color))
		{
			return false;
		}

		m_d3d12_frame_started = true;
		ID3D12DescriptorHeap* descriptor_heaps[] = { rhi->GetSrvHeap() };
		rhi->GetCommandList()->SetDescriptorHeaps(1, descriptor_heaps);
		BindD3D12GlobalResources();
		return true;
	}

	void DolasRHI::Present(TextureID scene_result_texture_id)
	{
		Texture* scene_result_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(scene_result_texture_id);
		DOLAS_RETURN_IF_NULL(scene_result_texture);

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		DOLAS_RETURN_IF_NULL(rhi);
		ID3D12GraphicsCommandList* command_list = rhi->GetCommandList();
		ID3D12Resource* back_buffer = rhi->GetCurrentBackBufferResource();
		ID3D12Resource* scene_result = scene_result_texture->GetD3D12Resource();
		DOLAS_RETURN_IF_NULL(command_list);
		DOLAS_RETURN_IF_NULL(back_buffer);
		DOLAS_RETURN_IF_NULL(scene_result);

		TransitionTexture(scene_result_texture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		TransitionResource(back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
		command_list->CopyResource(back_buffer, scene_result);
		TransitionResource(back_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE back_buffer_rtv = rhi->GetCurrentRtvHandle();
		command_list->OMSetRenderTargets(1, &back_buffer_rtv, FALSE, nullptr);
		RenderImGuiDrawData();

		if (rhi->EndFrame())
		{
			m_d3d12_frame_started = false;
		}
	}

	void DolasRHI::SetRenderTargetViewAndDepthStencilView(std::shared_ptr<RenderTargetView> d3d11_render_target_view, std::shared_ptr<DepthStencilView> depth_stencil_view)
	{
		if (d3d11_render_target_view)
		{
			std::vector<std::shared_ptr<RenderTargetView>> rtvs = { d3d11_render_target_view };
			SetRenderTargetViewAndDepthStencilView(rtvs, depth_stencil_view);
			return;
		}

		if (!m_d3d_immediate_context || !depth_stencil_view)
		{
			return;
		}

		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
		ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		d3d11_render_target_view_array[0] = d3d11_render_target_view ? d3d11_render_target_view->m_d3d_render_target_view : nullptr;
		m_d3d_immediate_context->OMSetRenderTargets(1, d3d11_render_target_view_array, depth_stencil_view->m_d3d_depth_stencil_view);
	}

	void DolasRHI::SetRenderTargetViewAndDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view, std::shared_ptr<DepthStencilView> depth_stencil_view)
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			const UINT rt_count = static_cast<UINT>(std::min<std::size_t>(d3d11_render_target_view.size(), 8));
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_rtvs[8] = {};
			m_current_render_target_count = rt_count;
			for (UINT i = 0; i < 8; ++i)
			{
				m_current_rtv_formats[i] = DXGI_FORMAT_UNKNOWN;
			}

			for (UINT i = 0; i < rt_count; ++i)
			{
				const auto& rtv = d3d11_render_target_view[i];
				if (!rtv || rtv->m_d3d12_render_target_view.ptr == 0)
				{
					continue;
				}

				Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(rtv->m_texture_id);
				DOLAS_CONTINUE_IF_NULL(texture);
				TransitionTexture(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
				d3d12_rtvs[i] = rtv->m_d3d12_render_target_view;
				if (ID3D12Resource* resource = texture->GetD3D12Resource())
				{
					D3D12_RESOURCE_DESC desc = resource->GetDesc();
					m_current_rtv_formats[i] = desc.Format;
				}
			}

			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_dsv = {};
			m_current_dsv_format = DXGI_FORMAT_UNKNOWN;
			if (depth_stencil_view && depth_stencil_view->m_d3d12_depth_stencil_view.ptr != 0)
			{
				Texture* depth_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(depth_stencil_view->m_texture_id);
				if (depth_texture)
				{
					TransitionTexture(depth_texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					if (ID3D12Resource* resource = depth_texture->GetD3D12Resource())
					{
						D3D12_RESOURCE_DESC desc = resource->GetDesc();
						switch (desc.Format)
						{
						case DXGI_FORMAT_R24G8_TYPELESS:
							m_current_dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
							break;
						case DXGI_FORMAT_R32_TYPELESS:
							m_current_dsv_format = DXGI_FORMAT_D32_FLOAT;
							break;
						default:
							m_current_dsv_format = desc.Format;
							break;
						}
					}
				}
				d3d12_dsv = depth_stencil_view->m_d3d12_depth_stencil_view;
			}

			command_list->OMSetRenderTargets(rt_count, rt_count > 0 ? d3d12_rtvs : nullptr, FALSE, d3d12_dsv.ptr ? &d3d12_dsv : nullptr);
		}

		if (!m_d3d_immediate_context)
		{
			return;
		}

		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
        if (d3d11_render_target_view.size() == 0) {
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, depth_stencil_view ? depth_stencil_view->m_d3d_depth_stencil_view : nullptr);
            return;
        }

        ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
            d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i]->m_d3d_render_target_view : nullptr;
		}
        std::size_t rt_count_sz = d3d11_render_target_view.size();
        if (rt_count_sz > k_max_render_targets) rt_count_sz = k_max_render_targets;
        if (rt_count_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) rt_count_sz = (std::size_t)(std::numeric_limits<UINT>::max)();
        m_d3d_immediate_context->OMSetRenderTargets((UINT)rt_count_sz, d3d11_render_target_view_array, depth_stencil_view ? depth_stencil_view->m_d3d_depth_stencil_view : nullptr);
	}

    void DolasRHI::SetRenderTargetViewWithoutDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view)
    {
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			const UINT rt_count = static_cast<UINT>(std::min<std::size_t>(d3d11_render_target_view.size(), 8));
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_rtvs[8] = {};
			m_current_render_target_count = rt_count;
			m_current_dsv_format = DXGI_FORMAT_UNKNOWN;
			for (UINT i = 0; i < 8; ++i)
			{
				m_current_rtv_formats[i] = DXGI_FORMAT_UNKNOWN;
			}

			for (UINT i = 0; i < rt_count; ++i)
			{
				const auto& rtv = d3d11_render_target_view[i];
				if (!rtv || rtv->m_d3d12_render_target_view.ptr == 0)
				{
					continue;
				}

				Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(rtv->m_texture_id);
				DOLAS_CONTINUE_IF_NULL(texture);
				TransitionTexture(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
				d3d12_rtvs[i] = rtv->m_d3d12_render_target_view;
				if (ID3D12Resource* resource = texture->GetD3D12Resource())
				{
					D3D12_RESOURCE_DESC desc = resource->GetDesc();
					m_current_rtv_formats[i] = desc.Format;
				}
			}

			command_list->OMSetRenderTargets(rt_count, rt_count > 0 ? d3d12_rtvs : nullptr, FALSE, nullptr);
		}

		if (!m_d3d_immediate_context)
		{
			return;
		}

		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
		if (d3d11_render_target_view.size() == 0) {
			return;
		}

		ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
			d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i]->m_d3d_render_target_view : nullptr;
		}
        std::size_t rt_count_sz = d3d11_render_target_view.size();
        if (rt_count_sz > k_max_render_targets) rt_count_sz = k_max_render_targets;
        if (rt_count_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) rt_count_sz = (std::size_t)(std::numeric_limits<UINT>::max)();
		m_d3d_immediate_context->OMSetRenderTargets((UINT)rt_count_sz, d3d11_render_target_view_array, nullptr);
    }

	void DolasRHI::ClearRenderTargetView(std::shared_ptr<RenderTargetView> rtv, const Float clear_color[4])
	{
		if (!rtv)
		{
			return;
		}

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list && rtv->m_d3d12_render_target_view.ptr != 0)
		{
			Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(rtv->m_texture_id);
			if (texture)
			{
				TransitionTexture(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
			command_list->ClearRenderTargetView(rtv->m_d3d12_render_target_view, clear_color, 0, nullptr);
		}

		if (m_d3d_immediate_context && rtv->m_d3d_render_target_view)
		{
			m_d3d_immediate_context->ClearRenderTargetView(rtv->m_d3d_render_target_view, clear_color);
		}
	}

	std::shared_ptr<RenderTargetView> DolasRHI::GetBackBufferRTV() const { return m_back_buffer_render_target_view; }

	std::shared_ptr<DepthStencilView> DolasRHI::CreateDepthStencilView(TextureID texture_id)
	{
		TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
		DOLAS_RETURN_NULL_IF_NULL(texture_manager);

		Texture* texture = texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_NULL_IF_NULL(texture);

		std::shared_ptr<DepthStencilView> depth_stencil_view = std::make_shared<DepthStencilView>();
		depth_stencil_view->m_texture_id = texture_id;
		depth_stencil_view->m_d3d12_depth_stencil_view = texture->GetD3D12DsvHandle();

		ID3D11Texture2D* d3d_texture_2d = texture->GetD3DTexture2D();
		if (m_d3d_device && d3d_texture_2d)
		{
			D3D11_TEXTURE2D_DESC texture_desc = {};
			d3d_texture_2d->GetDesc(&texture_desc);

			DXGI_FORMAT dsv_format = texture_desc.Format;
			switch (texture_desc.Format)
			{
			case DXGI_FORMAT_R24G8_TYPELESS:
				dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				break;
			case DXGI_FORMAT_R32_TYPELESS:
				dsv_format = DXGI_FORMAT_D32_FLOAT;
				break;
			default:
				break;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
			dsv_desc.Format = dsv_format;
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
			HR(g_dolas_engine.m_rhi->m_d3d_device->CreateDepthStencilView(d3d_texture_2d, &dsv_desc, &depth_stencil_view->m_d3d_depth_stencil_view));
		}

		return depth_stencil_view;
	}

	void DolasRHI::ClearDepthStencilView(std::shared_ptr<DepthStencilView> dsv, const DepthClearParams& depth_clear_params, const StencilClearParams& stencil_clear_params)
	{
		if (!dsv)
		{
			return;
		}

		UINT clear_flags = 0;
		if (depth_clear_params.enable)
		{
			clear_flags |= D3D11_CLEAR_DEPTH;
		}
		if (stencil_clear_params.enable)
		{
			clear_flags |= D3D11_CLEAR_STENCIL;
		}

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list && dsv->m_d3d12_depth_stencil_view.ptr != 0)
		{
			Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(dsv->m_texture_id);
			if (texture)
			{
				TransitionTexture(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			}

			D3D12_CLEAR_FLAGS d3d12_clear_flags = static_cast<D3D12_CLEAR_FLAGS>(0);
			if (depth_clear_params.enable)
			{
				d3d12_clear_flags = static_cast<D3D12_CLEAR_FLAGS>(d3d12_clear_flags | D3D12_CLEAR_FLAG_DEPTH);
			}
			if (stencil_clear_params.enable)
			{
				d3d12_clear_flags = static_cast<D3D12_CLEAR_FLAGS>(d3d12_clear_flags | D3D12_CLEAR_FLAG_STENCIL);
			}
			command_list->ClearDepthStencilView(dsv->m_d3d12_depth_stencil_view, d3d12_clear_flags, depth_clear_params.clear_value, static_cast<UINT8>(stencil_clear_params.clear_value), 0, nullptr);
		}

		if (m_d3d_immediate_context && dsv->m_d3d_depth_stencil_view)
		{
			m_d3d_immediate_context->ClearDepthStencilView(dsv->m_d3d_depth_stencil_view, clear_flags, depth_clear_params.clear_value, stencil_clear_params.clear_value);
		}
	}

    void DolasRHI::SetViewPort(const ViewPort& viewport)
	{
		D3D11_VIEWPORT d3d_viewport = {};
		d3d_viewport.TopLeftX = viewport.m_top_left_x;
		d3d_viewport.TopLeftY = viewport.m_top_left_y;
		d3d_viewport.Width = viewport.m_width;
		d3d_viewport.Height = viewport.m_height;
		d3d_viewport.MinDepth = viewport.m_min_depth;
		d3d_viewport.MaxDepth = viewport.m_max_depth;
		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->RSSetViewports(1, &d3d_viewport);
		}

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			D3D12_VIEWPORT d3d12_viewport = {};
			d3d12_viewport.TopLeftX = viewport.m_top_left_x;
			d3d12_viewport.TopLeftY = viewport.m_top_left_y;
			d3d12_viewport.Width = viewport.m_width;
			d3d12_viewport.Height = viewport.m_height;
			d3d12_viewport.MinDepth = viewport.m_min_depth;
			d3d12_viewport.MaxDepth = viewport.m_max_depth;
			command_list->RSSetViewports(1, &d3d12_viewport);

			D3D12_RECT scissor_rect = {};
			scissor_rect.left = static_cast<LONG>(viewport.m_top_left_x);
			scissor_rect.top = static_cast<LONG>(viewport.m_top_left_y);
			scissor_rect.right = static_cast<LONG>(viewport.m_top_left_x + viewport.m_width);
			scissor_rect.bottom = static_cast<LONG>(viewport.m_top_left_y + viewport.m_height);
			command_list->RSSetScissorRects(1, &scissor_rect);
		}
	}
	
	void DolasRHI::SetRasterizerState(RasterizerStateType type)
	{
		m_current_rasterizer_state_type = type;
		RasterizerState& rasterizer_state = m_rasterizer_states[static_cast<UInt>(type)];
		if (!m_d3d_immediate_context)
		{
			return;
		}
		if (rasterizer_state.m_d3d_rasterizer_state == nullptr)
		{
			rasterizer_state.m_d3d_rasterizer_state = CreateRasterizerState(type);
		}

		if (rasterizer_state.m_d3d_rasterizer_state == nullptr)
		{
			return;
		}
		m_d3d_immediate_context->RSSetState(rasterizer_state.m_d3d_rasterizer_state);
	}

    void DolasRHI::SetDepthStencilState(DepthStencilStateType type)
    {
		m_current_depth_stencil_state_type = type;
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			command_list->OMSetStencilRef(m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[type].second);
		}
		if (!m_d3d_immediate_context)
		{
			return;
		}
		const DepthStencilState& depth_stencil_state = GetOrCreateDepthStencilState(type);
        m_d3d_immediate_context->OMSetDepthStencilState(depth_stencil_state.m_d3d_depth_stencil_state, depth_stencil_state.m_stencil_ref_value);
    }

    void DolasRHI::SetBlendState(BlendStateType type)
    {
		m_current_blend_state_type = type;
		if (!m_d3d_immediate_context)
		{
			return;
		}
		BlendState& blend_state = m_blend_states[static_cast<UInt>(type)];
		if (blend_state.m_d3d_blend_state == nullptr)
		{
			blend_state.m_d3d_blend_state = CreateBlendState(type);
		}

		if (blend_state.m_d3d_blend_state == nullptr)
		{
			return;
		}
        m_d3d_immediate_context->OMSetBlendState(blend_state.m_d3d_blend_state, nullptr, 0xFFFFFFFF);
    }

	Bool DolasRHI::BindVertexContext(std::shared_ptr<VertexContext> vertex_context, ID3D11ClassInstance* const* class_instances/* = nullptr*/, UINT num_class_instances/* = 0*/)
	{
		DOLAS_RETURN_FALSE_IF_NULL(vertex_context);
		m_current_vertex_context = vertex_context;

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list && m_d3d12_root_signature)
		{
			command_list->SetGraphicsRootSignature(m_d3d12_root_signature);
			BindD3D12GlobalResources();
			ID3D12Resource* global_constant_buffer = vertex_context->GetD3D12GlobalConstantBuffer();
			if (!global_constant_buffer)
			{
				global_constant_buffer = m_d3d12_dummy_constant_buffer;
			}
			if (global_constant_buffer)
			{
				command_list->SetGraphicsRootConstantBufferView(kRootVSGlobalCBV, global_constant_buffer->GetGPUVirtualAddress());
			}
			vertex_context->ConvertTextureIDMapToSRVMap();
			BindD3D12SrvTable(vertex_context, false);
		}

		if (!m_d3d_immediate_context)
		{
			m_current_vs_bytecode = vertex_context->GetShaderBytecode();
			return true;
		}

		/* 1. Set shader to context */
		m_d3d_immediate_context->VSSetShader(vertex_context->GetD3DVertexShader(), class_instances, num_class_instances);

		/* 2. Set SRVs */
		// Convert 'm_slot_to_texture_map' to 'm_slot_to_srv_map'
		vertex_context->ConvertTextureIDMapToSRVMap();
		// Bind the SRV to the corresponding slot based on the 'm_slot_to_srv_map'
		const auto& srvs = vertex_context->GetSlotToSRVMap();
		for (auto srv_iter : srvs)
		{
            const std::size_t slot_sz = (std::size_t)srv_iter.first;
            const UINT slot = (slot_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) ? (std::numeric_limits<UINT>::max)() : (UINT)slot_sz;
			m_d3d_immediate_context->VSSetShaderResources(slot, 1, &(srv_iter.second));
		}

		/* 3. Set Samplers(TODO) */

		/* 4. Set Constant Buffer */
		// Bind Constant Buffer to context
		ID3D11Buffer* global_constant_buffer = vertex_context->GetGlobalConstantBuffer();
        if (global_constant_buffer)
        {
            m_d3d_immediate_context->VSSetConstantBuffers(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1, 1, &global_constant_buffer);
        }

		// Update Constant Buffer Data（将 ShaderContext 中预打包好的全局常量拷贝到 GPU CB）
		const std::vector<uint8_t>& cb_data = vertex_context->GetGlobalConstantBufferData();
		if (global_constant_buffer && !cb_data.empty())
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(m_d3d_immediate_context->Map(global_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
			memcpy_s(mappedData.pData, cb_data.size(), cb_data.data(), cb_data.size());
			m_d3d_immediate_context->Unmap(global_constant_buffer, 0);
		}

		/* 5. Cache VS bytecode for InputLayout creating later */
		m_current_vs_bytecode = vertex_context->GetShaderBytecode();

		return true;
	}

	// PixelContext
	Bool DolasRHI::BindPixelContext(std::shared_ptr<PixelContext> pixel_context, ID3D11ClassInstance* const* class_instances/* = nullptr*/, UINT num_class_instances/* = 0*/)
	{
		DOLAS_RETURN_FALSE_IF_NULL(pixel_context);
		m_current_pixel_context = pixel_context;

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list && m_d3d12_root_signature)
		{
			command_list->SetGraphicsRootSignature(m_d3d12_root_signature);
			BindD3D12GlobalResources();
			ID3D12Resource* global_constant_buffer = pixel_context->GetD3D12GlobalConstantBuffer();
			if (!global_constant_buffer)
			{
				global_constant_buffer = m_d3d12_dummy_constant_buffer;
			}
			if (global_constant_buffer)
			{
				command_list->SetGraphicsRootConstantBufferView(kRootPSGlobalCBV, global_constant_buffer->GetGPUVirtualAddress());
			}
			pixel_context->ConvertTextureIDMapToSRVMap();
			BindD3D12SrvTable(pixel_context, true);
		}

		if (!m_d3d_immediate_context)
		{
			return true;
		}

		/* 1. Set shader to context */ 
		m_d3d_immediate_context->PSSetShader(pixel_context->GetD3DPixelShader(), class_instances, num_class_instances);
		
		/* 2. Set SRVs */
		// Convert 'm_slot_to_texture_map' to 'm_slot_to_srv_map'
		pixel_context->ConvertTextureIDMapToSRVMap();
		// Bind the SRV to the corresponding slot based on the 'm_slot_to_srv_map'
		const auto& srvs = pixel_context->GetSlotToSRVMap();
		for (auto srv_iter : srvs)
		{
            const std::size_t slot_sz = (std::size_t)srv_iter.first;
            const UINT slot = (slot_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) ? (std::numeric_limits<UINT>::max)() : (UINT)slot_sz;
			m_d3d_immediate_context->PSSetShaderResources(slot, 1, &(srv_iter.second));
		}

		/* 3. Set Samplers(TODO) */

		/* 4. Set Constant Buffer */
		// Bind Constant Buffer to context
		ID3D11Buffer* global_constant_buffer = pixel_context->GetGlobalConstantBuffer();
        if (global_constant_buffer)
        {
            m_d3d_immediate_context->PSSetConstantBuffers(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1, 1, &global_constant_buffer);
        }
		// Update Constant Buffer Data（将 ShaderContext 中预打包好的全局常量拷贝到 GPU CB）
		const std::vector<uint8_t>& cb_data = pixel_context->GetGlobalConstantBufferData();
		if (global_constant_buffer && !cb_data.empty())
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(m_d3d_immediate_context->Map(global_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
			memcpy_s(mappedData.pData, cb_data.size(), cb_data.data(), cb_data.size());
			m_d3d_immediate_context->Unmap(global_constant_buffer, 0);
		}

		return true;
	}

	void DolasRHI::SetPrimitiveTopology(PrimitiveTopology primitive_topology)
	{
		m_current_primitive_topology = primitive_topology;
		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->IASetPrimitiveTopology(m_d3d11_state_cache->primitive_topology[static_cast<UInt>(primitive_topology)]);
		}

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			command_list->IASetPrimitiveTopology(m_d3d11_state_cache->d3d12_primitive_topology[static_cast<UInt>(primitive_topology)]);
		}
	}

	void DolasRHI::SetInputLayout(InputLayoutType input_layout_type, const void* vs_blob, size_t bytecode_length)
	{
		if (!m_d3d_immediate_context)
		{
			return;
		}
		std::shared_ptr<InputLayout> input_layout = CreateInputLayout(input_layout_type, vs_blob, bytecode_length);
		m_d3d_immediate_context->IASetInputLayout(input_layout->m_d3d_input_layout);
	}

	void DolasRHI::SetVertexBuffers(const std::vector<BufferID>& vertex_buffer_ids, const std::vector<UInt>& vertex_strides, const std::vector<UInt>& vertex_offsets)
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			std::vector<D3D12_VERTEX_BUFFER_VIEW> d3d12_buffer_views;
			for (std::size_t i = 0; i < vertex_buffer_ids.size(); i++)
			{
				Buffer* buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(vertex_buffer_ids[i]);
				DOLAS_CONTINUE_IF_NULL(buffer);
				ID3D12Resource* resource = buffer->GetD3D12Resource();
				DOLAS_CONTINUE_IF_NULL(resource);

				D3D12_VERTEX_BUFFER_VIEW view = {};
				view.BufferLocation = resource->GetGPUVirtualAddress();
				view.SizeInBytes = buffer->GetSize();
				view.StrideInBytes = i < vertex_strides.size() ? vertex_strides[i] : buffer->GetStride();
				d3d12_buffer_views.push_back(view);
			}

			if (!d3d12_buffer_views.empty())
			{
				command_list->IASetVertexBuffers(0, static_cast<UINT>(d3d12_buffer_views.size()), d3d12_buffer_views.data());
			}
		}

		if (!m_d3d_immediate_context)
		{
			return;
		}

		std::vector<ID3D11Buffer*> d3d11_buffers;
		for (std::size_t i = 0; i < vertex_buffer_ids.size(); i++)
		{
			Buffer* buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(vertex_buffer_ids[i]);
			d3d11_buffers.push_back(buffer->GetBuffer());
		}
        std::size_t vb_count_sz = d3d11_buffers.size();
        if (vb_count_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) vb_count_sz = (std::size_t)(std::numeric_limits<UINT>::max)();
		m_d3d_immediate_context->IASetVertexBuffers(0, (UINT)vb_count_sz, d3d11_buffers.data(), vertex_strides.data(), vertex_offsets.data());
	}

	void DolasRHI::SetIndexBuffer(BufferID index_buffer_id)
	{
		Buffer* buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(index_buffer_id);
		DOLAS_RETURN_IF_NULL(buffer);

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list && buffer->GetD3D12Resource())
		{
			D3D12_INDEX_BUFFER_VIEW index_view = {};
			index_view.BufferLocation = buffer->GetD3D12Resource()->GetGPUVirtualAddress();
			index_view.SizeInBytes = buffer->GetSize();
			index_view.Format = DXGI_FORMAT_R32_UINT;
			command_list->IASetIndexBuffer(&index_view);
		}

		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->IASetIndexBuffer(buffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		}
	}

	void DolasRHI::DrawIndexed(UInt index_count)
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (command_list)
		{
			command_list->DrawIndexedInstanced(index_count, 1, 0, 0, 0);
		}

		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->DrawIndexed(index_count, 0, 0);
		}
	}

	void DolasRHI::DrawRenderPrimitive(RenderPrimitiveID render_primitive_id)
	{
		RenderPrimitive* render_primitive = g_dolas_engine.m_render_primitive_manager->GetRenderPrimitiveByID(render_primitive_id);
		DOLAS_RETURN_IF_NULL(render_primitive);

		if (!m_current_vs_bytecode.IsValid())
		{
			return;
		}

		SetInputLayout(render_primitive->m_input_layout_type, m_current_vs_bytecode.data, m_current_vs_bytecode.size);

		SetPrimitiveTopology(render_primitive->m_topology);

		SetVertexBuffers(render_primitive->m_vertex_buffer_ids, render_primitive->m_vertex_strides, render_primitive->m_vertex_offsets);

		SetIndexBuffer(render_primitive->m_index_buffer_id);

		if (ID3D12PipelineState* pso = GetOrCreateD3D12PipelineState(render_primitive))
		{
			RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
			ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
			if (command_list)
			{
				command_list->SetPipelineState(pso);
			}
		}

		DrawIndexed(render_primitive->m_index_count);
	}

	void DolasRHI::VSSetConstantBuffers()
	{
		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->VSSetConstantBuffers(0, 1, &m_d3d_per_view_parameters_buffer);
			m_d3d_immediate_context->VSSetConstantBuffers(1, 1, &m_d3d_per_frame_parameters_buffer);
			m_d3d_immediate_context->VSSetConstantBuffers(2, 1, &m_d3d_per_object_parameters_buffer);
		}
	}

	void DolasRHI::PSSetConstantBuffers()
	{
		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->PSSetConstantBuffers(0, 1, &m_d3d_per_view_parameters_buffer);
			m_d3d_immediate_context->PSSetConstantBuffers(1, 1, &m_d3d_per_frame_parameters_buffer);
			m_d3d_immediate_context->PSSetConstantBuffers(2, 1, &m_d3d_per_object_parameters_buffer);
		}
	}

	ID3D11ShaderResourceView* DolasRHI::CreateShaderResourceView(ID3D11Resource* resource)
	{
		if (!m_d3d_device)
		{
			return nullptr;
		}
		ID3D11ShaderResourceView* shader_resource_view = nullptr;
		HR(m_d3d_device->CreateShaderResourceView(resource, nullptr, &shader_resource_view));

		return shader_resource_view;
	}
	
	void DolasRHI::UpdatePerFrameParameters()
	{
		PerFrameConstantBuffer per_frame_constant_buffer;
		per_frame_constant_buffer.light_direction_intensity = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);
		per_frame_constant_buffer.light_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		if (m_d3d_immediate_context && m_d3d_per_frame_parameters_buffer)
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(m_d3d_immediate_context->Map(m_d3d_per_frame_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

			memcpy_s(mappedData.pData, sizeof(per_frame_constant_buffer), &per_frame_constant_buffer, sizeof(per_frame_constant_buffer));
			m_d3d_immediate_context->Unmap(m_d3d_per_frame_parameters_buffer, 0);
		}
		UpdateD3D12UploadBuffer(m_d3d12_per_frame_parameters_buffer, &per_frame_constant_buffer, sizeof(per_frame_constant_buffer));
	}

	void DolasRHI::UpdatePerViewParameters(RenderCamera* render_camera)
	{
		PerViewConstantBuffer per_view_constant_buffer;
		per_view_constant_buffer.view = render_camera->GetViewMatrix();
		per_view_constant_buffer.proj = render_camera->GetProjectionMatrix();
		per_view_constant_buffer.camera_position = Vector4(render_camera->GetPosition(), 1.0f);

		if (m_d3d_immediate_context && m_d3d_per_view_parameters_buffer)
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(m_d3d_immediate_context->Map(m_d3d_per_view_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
			memcpy_s(mappedData.pData, sizeof(per_view_constant_buffer), &per_view_constant_buffer, sizeof(per_view_constant_buffer));
			m_d3d_immediate_context->Unmap(m_d3d_per_view_parameters_buffer, 0);
		}
		UpdateD3D12UploadBuffer(m_d3d12_per_view_parameters_buffer, &per_view_constant_buffer, sizeof(per_view_constant_buffer));
	}

	void DolasRHI::UpdatePerObjectParameters(Pose pose)
	{
		PerObjectConstantBuffer per_object_constant_buffer;

		// 从 Pose 构建世界矩阵
		// 世界矩阵 = 缩放矩阵 * 旋转矩阵 * 平移矩阵

		// 提取 Pose 数据
		Vector3 position = pose.m_postion;  // 注意：原始代码中拼写是 m_postion
		Quaternion quat = pose.m_rotation;     // 四元数 (x, y, z, w)
		Vector3 scale = pose.m_scale;

		// 归一化四元数（确保是单位四元数）
		Float quat_length = sqrt(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w);
		if (quat_length > 0.0001f)
		{
			quat.x /= quat_length;
			quat.y /= quat_length;
			quat.z /= quat_length;
			quat.w /= quat_length;
		}

		// 从四元数构建旋转矩阵
		Float xx = quat.x * quat.x;
		Float yy = quat.y * quat.y;
		Float zz = quat.z * quat.z;
		Float xy = quat.x * quat.y;
		Float xz = quat.x * quat.z;
		Float yz = quat.y * quat.z;
		Float wx = quat.w * quat.x;
		Float wy = quat.w * quat.y;
		Float wz = quat.w * quat.z;

		Matrix4x4 ratation_mat{
			1.0f - 2.0f * (yy + zz),     2.0f * (xy - wz),         2.0f * (xz + wy),         0.0f,
			2.0f * (xy + wz),           1.0f - 2.0f * (xx + zz),   2.0f * (yz - wx),       0.0f,
			2.0f * (xz - wy),           2.0f * (yz + wx),         1.0f - 2.0f * (xx + yy), 0.0f ,
			0.0f,                       0.0f,                     0.0f,                     1.0f
		};

		Matrix4x4 scale_mat{
			scale.x, 0.0f,     0.0f,     0.0f,
			0.0f,     scale.y, 0.0f,     0.0f,
			0.0f,     0.0f,     scale.z, 0.0f,
			0.0f,     0.0f,     0.0f,     1.0f
		};

		Matrix4x4 trans_mat{
			1.0f, 0.0f, 0.0f, position.x,
			0.0f, 1.0f, 0.0f, position.y,
			0.0f, 0.0f, 1.0f, position.z,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		per_object_constant_buffer.world = trans_mat * ratation_mat * scale_mat;

		if (m_d3d_immediate_context && m_d3d_per_object_parameters_buffer)
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			HR(m_d3d_immediate_context->Map(m_d3d_per_object_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

			memcpy_s(mappedData.pData, sizeof(per_object_constant_buffer), &per_object_constant_buffer, sizeof(per_object_constant_buffer));
			m_d3d_immediate_context->Unmap(m_d3d_per_object_parameters_buffer, 0);
		}
		UpdateD3D12UploadBuffer(m_d3d12_per_object_parameters_buffer, &per_object_constant_buffer, sizeof(per_object_constant_buffer));
	}

	bool DolasRHI::InitializeD3D11CompatibilityDevice()
	{
		IDXGIAdapter* best_adapter = DXGIHelper::SelectBestAdapter();
		if (!best_adapter)
		{
			LOG_WARN("Failed to select best adapter for optional D3D11 compatibility device.");
			return false;
		}

		DXGIHelper::PrintAdapterInfo(best_adapter);

		D3D_FEATURE_LEVEL feature_levels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};
		D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

		UINT d3d_device_create_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		d3d_device_create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr = D3D11CreateDevice(
			best_adapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			d3d_device_create_flags,
			feature_levels,
			ARRAYSIZE(feature_levels),
			D3D11_SDK_VERSION,
			&m_d3d_device,
			&feature_level,
			&m_d3d_immediate_context);

#if defined(DEBUG) || defined(_DEBUG)
		if (FAILED(hr) && (d3d_device_create_flags & D3D11_CREATE_DEVICE_DEBUG))
		{
			LOG_INFO("D3D11 debug layer unavailable for compatibility device, retrying without it.");
			d3d_device_create_flags &= ~D3D11_CREATE_DEVICE_DEBUG;
			hr = D3D11CreateDevice(
				best_adapter,
				D3D_DRIVER_TYPE_UNKNOWN,
				nullptr,
				d3d_device_create_flags,
				feature_levels,
				ARRAYSIZE(feature_levels),
				D3D11_SDK_VERSION,
				&m_d3d_device,
				&feature_level,
				&m_d3d_immediate_context);
		}
#endif

		best_adapter->Release();
		if (FAILED(hr))
		{
			LOG_WARN("Failed to create optional D3D11 compatibility device. HRESULT: 0x{0:X}", hr);
			return false;
		}

		if (m_d3d_immediate_context)
		{
			m_d3d_immediate_context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), reinterpret_cast<void**>(&m_d3d_user_annotation));
		}

		LOG_INFO("Created headless D3D11 compatibility device. Feature Level: {0:#x}", static_cast<int>(feature_level));
		return true;
	}

	bool DolasRHI::InitializeD3D12CompatibilityResources()
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12Device* device = rhi ? rhi->GetDevice() : nullptr;
		if (!device)
		{
			LOG_ERROR("D3D12 device is null while initializing DolasRHI compatibility resources.");
			return false;
		}

		InitializeRasterizerStateCreateDesc();
		InitializeDepthStencilStateCreateDesc();
		InitializeBlendStateCreateDesc();
		InitializePrimitiveTopology();
		InitializeInputLayoutDescs();

		if (!CreateD3D12UploadBuffer(device, sizeof(PerViewConstantBuffer), nullptr, &m_d3d12_per_view_parameters_buffer) ||
			!CreateD3D12UploadBuffer(device, sizeof(PerFrameConstantBuffer), nullptr, &m_d3d12_per_frame_parameters_buffer) ||
			!CreateD3D12UploadBuffer(device, sizeof(PerObjectConstantBuffer), nullptr, &m_d3d12_per_object_parameters_buffer) ||
			!CreateD3D12UploadBuffer(device, 256, nullptr, &m_d3d12_dummy_constant_buffer))
		{
			return false;
		}

		D3D12_DESCRIPTOR_RANGE srv_ranges[2] = {};
		srv_ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srv_ranges[0].NumDescriptors = kD3D12SrvTableSize;
		srv_ranges[0].BaseShaderRegister = 0;
		srv_ranges[0].RegisterSpace = 0;
		srv_ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		srv_ranges[1] = srv_ranges[0];

		D3D12_ROOT_PARAMETER root_parameters[7] = {};
		root_parameters[kRootPerViewCBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_parameters[kRootPerViewCBV].Descriptor.ShaderRegister = 0;
		root_parameters[kRootPerViewCBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		root_parameters[kRootPerFrameCBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_parameters[kRootPerFrameCBV].Descriptor.ShaderRegister = 1;
		root_parameters[kRootPerFrameCBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		root_parameters[kRootPerObjectCBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_parameters[kRootPerObjectCBV].Descriptor.ShaderRegister = 2;
		root_parameters[kRootPerObjectCBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		root_parameters[kRootVSGlobalCBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_parameters[kRootVSGlobalCBV].Descriptor.ShaderRegister = 13;
		root_parameters[kRootVSGlobalCBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		root_parameters[kRootPSGlobalCBV].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		root_parameters[kRootPSGlobalCBV].Descriptor.ShaderRegister = 13;
		root_parameters[kRootPSGlobalCBV].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		root_parameters[kRootVSSrvTable].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameters[kRootVSSrvTable].DescriptorTable.NumDescriptorRanges = 1;
		root_parameters[kRootVSSrvTable].DescriptorTable.pDescriptorRanges = &srv_ranges[0];
		root_parameters[kRootVSSrvTable].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		root_parameters[kRootPSSrvTable].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		root_parameters[kRootPSSrvTable].DescriptorTable.NumDescriptorRanges = 1;
		root_parameters[kRootPSSrvTable].DescriptorTable.pDescriptorRanges = &srv_ranges[1];
		root_parameters[kRootPSSrvTable].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC samplers[8] = {};
		auto configure_sampler = [](D3D12_STATIC_SAMPLER_DESC& sampler, UINT shader_register, D3D12_TEXTURE_ADDRESS_MODE address_mode)
		{
			sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			sampler.AddressU = address_mode;
			sampler.AddressV = address_mode;
			sampler.AddressW = address_mode;
			sampler.MipLODBias = 0.0f;
			sampler.MaxAnisotropy = 1;
			sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			sampler.MinLOD = 0.0f;
			sampler.MaxLOD = D3D12_FLOAT32_MAX;
			sampler.ShaderRegister = shader_register;
			sampler.RegisterSpace = 0;
			sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		};
		for (UINT i = 0; i < 5; ++i)
		{
			configure_sampler(samplers[i], i, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		}
		configure_sampler(samplers[5], 13, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		configure_sampler(samplers[6], 14, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		configure_sampler(samplers[7], 15, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
		root_signature_desc.NumParameters = ARRAYSIZE(root_parameters);
		root_signature_desc.pParameters = root_parameters;
		root_signature_desc.NumStaticSamplers = ARRAYSIZE(samplers);
		root_signature_desc.pStaticSamplers = samplers;
		root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* signature_blob = nullptr;
		ID3DBlob* error_blob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature_blob, &error_blob);
		if (FAILED(hr))
		{
			if (error_blob)
			{
				LOG_ERROR("Failed to serialize D3D12 root signature: {0}", static_cast<const char*>(error_blob->GetBufferPointer()));
			}
			SafeRelease(error_blob);
			return false;
		}
		SafeRelease(error_blob);

		hr = device->CreateRootSignature(
			0,
			signature_blob->GetBufferPointer(),
			signature_blob->GetBufferSize(),
			IID_PPV_ARGS(&m_d3d12_root_signature));
		SafeRelease(signature_blob);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create D3D12 root signature! HRESULT: 0x{0:X}", hr);
			return false;
		}

		return true;
	}

	void DolasRHI::InitializeRasterizerStateCreateDesc()
	{
		D3D11_RASTERIZER_DESC solid_none_cull_desc = {};
		solid_none_cull_desc.FillMode = D3D11_FILL_SOLID;
		solid_none_cull_desc.CullMode = D3D11_CULL_NONE;
		solid_none_cull_desc.FrontCounterClockwise = FALSE;
		solid_none_cull_desc.DepthBias = 0;
		solid_none_cull_desc.DepthBiasClamp = 0.0f;
		solid_none_cull_desc.SlopeScaledDepthBias = 0.0f;
		solid_none_cull_desc.DepthClipEnable = TRUE;
		solid_none_cull_desc.ScissorEnable = FALSE;
		solid_none_cull_desc.MultisampleEnable = FALSE;
		solid_none_cull_desc.AntialiasedLineEnable = FALSE;
		m_d3d11_state_cache->rasterizer_state_create_desc[RasterizerStateType_SolidNoneCull] = solid_none_cull_desc;
		D3D12_RASTERIZER_DESC d3d12_solid_none_cull_desc = {};
		d3d12_solid_none_cull_desc.FillMode = D3D12_FILL_MODE_SOLID;
		d3d12_solid_none_cull_desc.CullMode = D3D12_CULL_MODE_NONE;
		d3d12_solid_none_cull_desc.FrontCounterClockwise = FALSE;
		d3d12_solid_none_cull_desc.DepthClipEnable = TRUE;
		d3d12_solid_none_cull_desc.MultisampleEnable = FALSE;
		d3d12_solid_none_cull_desc.AntialiasedLineEnable = FALSE;
		d3d12_solid_none_cull_desc.ForcedSampleCount = 0;
		d3d12_solid_none_cull_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		m_d3d11_state_cache->d3d12_rasterizer_state_create_desc[RasterizerStateType_SolidNoneCull] = d3d12_solid_none_cull_desc;

		D3D11_RASTERIZER_DESC solid_back_cull_desc = {};
		solid_back_cull_desc.FillMode = D3D11_FILL_SOLID;
		solid_back_cull_desc.CullMode = D3D11_CULL_BACK;
		solid_back_cull_desc.FrontCounterClockwise = FALSE;
		solid_back_cull_desc.DepthBias = 0;
		solid_back_cull_desc.DepthBiasClamp = 0.0f;
		solid_back_cull_desc.SlopeScaledDepthBias = 0.0f;
		solid_back_cull_desc.DepthClipEnable = TRUE;
		solid_back_cull_desc.ScissorEnable = FALSE;
		solid_back_cull_desc.MultisampleEnable = FALSE;
		solid_back_cull_desc.AntialiasedLineEnable = FALSE;
		m_d3d11_state_cache->rasterizer_state_create_desc[RasterizerStateType_SolidBackCull] = solid_back_cull_desc;
		D3D12_RASTERIZER_DESC d3d12_solid_back_cull_desc = d3d12_solid_none_cull_desc;
		d3d12_solid_back_cull_desc.CullMode = D3D12_CULL_MODE_BACK;
		m_d3d11_state_cache->d3d12_rasterizer_state_create_desc[RasterizerStateType_SolidBackCull] = d3d12_solid_back_cull_desc;

		D3D11_RASTERIZER_DESC solid_front_cull_desc = {};
		solid_front_cull_desc.FillMode = D3D11_FILL_SOLID;
		solid_front_cull_desc.CullMode = D3D11_CULL_FRONT;
		solid_front_cull_desc.FrontCounterClockwise = FALSE;
		solid_front_cull_desc.DepthBias = 0;
		solid_front_cull_desc.DepthBiasClamp = 0.0f;
		solid_front_cull_desc.SlopeScaledDepthBias = 0.0f;
		solid_front_cull_desc.DepthClipEnable = TRUE;
		solid_front_cull_desc.ScissorEnable = FALSE;
		solid_front_cull_desc.MultisampleEnable = FALSE;
		solid_front_cull_desc.AntialiasedLineEnable = FALSE;
		m_d3d11_state_cache->rasterizer_state_create_desc[RasterizerStateType_SolidFrontCull] = solid_front_cull_desc;
		D3D12_RASTERIZER_DESC d3d12_solid_front_cull_desc = d3d12_solid_none_cull_desc;
		d3d12_solid_front_cull_desc.CullMode = D3D12_CULL_MODE_FRONT;
		m_d3d11_state_cache->d3d12_rasterizer_state_create_desc[RasterizerStateType_SolidFrontCull] = d3d12_solid_front_cull_desc;

		D3D11_RASTERIZER_DESC wireframe_desc = {};
		wireframe_desc.FillMode = D3D11_FILL_WIREFRAME;
		wireframe_desc.CullMode = D3D11_CULL_BACK;
		wireframe_desc.FrontCounterClockwise = FALSE;
		wireframe_desc.DepthBias = 0;
		wireframe_desc.DepthBiasClamp = 0.0f;
		wireframe_desc.SlopeScaledDepthBias = 0.0f;
		wireframe_desc.DepthClipEnable = TRUE;
		wireframe_desc.ScissorEnable = FALSE;
		wireframe_desc.MultisampleEnable = FALSE;
		wireframe_desc.AntialiasedLineEnable = FALSE;
		m_d3d11_state_cache->rasterizer_state_create_desc[RasterizerStateType_Wireframe] = wireframe_desc;
		D3D12_RASTERIZER_DESC d3d12_wireframe_desc = d3d12_solid_back_cull_desc;
		d3d12_wireframe_desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		m_d3d11_state_cache->d3d12_rasterizer_state_create_desc[RasterizerStateType_Wireframe] = d3d12_wireframe_desc;
	}

	void DolasRHI::InitializeDepthStencilStateCreateDesc()
	{
		D3D11_DEPTH_STENCIL_DESC depth_write_less_stencil_read_static_desc = {};
		depth_write_less_stencil_read_static_desc.DepthEnable = TRUE;
		depth_write_less_stencil_read_static_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_write_less_stencil_read_static_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_write_less_stencil_read_static_desc.StencilEnable = TRUE;
		depth_write_less_stencil_read_static_desc.StencilReadMask = 0xFF;
		depth_write_less_stencil_read_static_desc.StencilWriteMask = 0xFF;
		depth_write_less_stencil_read_static_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_write_less_stencil_read_static_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depth_write_less_stencil_read_static_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depth_write_less_stencil_read_static_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_write_less_stencil_read_static_desc.BackFace = depth_write_less_stencil_read_static_desc.FrontFace;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].first = depth_write_less_stencil_read_static_desc;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].second = StencilMaskEnum_Static;
		D3D12_DEPTH_STENCIL_DESC d3d12_depth_write_less_stencil_write_static_desc = {};
		d3d12_depth_write_less_stencil_write_static_desc.DepthEnable = TRUE;
		d3d12_depth_write_less_stencil_write_static_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3d12_depth_write_less_stencil_write_static_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3d12_depth_write_less_stencil_write_static_desc.StencilEnable = TRUE;
		d3d12_depth_write_less_stencil_write_static_desc.StencilReadMask = 0xFF;
		d3d12_depth_write_less_stencil_write_static_desc.StencilWriteMask = 0xFF;
		d3d12_depth_write_less_stencil_write_static_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3d12_depth_write_less_stencil_write_static_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3d12_depth_write_less_stencil_write_static_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3d12_depth_write_less_stencil_write_static_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		d3d12_depth_write_less_stencil_write_static_desc.BackFace = d3d12_depth_write_less_stencil_write_static_desc.FrontFace;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].first = d3d12_depth_write_less_stencil_write_static_desc;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].second = StencilMaskEnum_Static;

		D3D11_DEPTH_STENCIL_DESC depth_disabled_stencil_disable_desc = {};
		depth_disabled_stencil_disable_desc.DepthEnable = FALSE;
		depth_disabled_stencil_disable_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_disabled_stencil_disable_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_disabled_stencil_disable_desc.StencilEnable = FALSE;
		depth_disabled_stencil_disable_desc.StencilReadMask = 0xFF;
		depth_disabled_stencil_disable_desc.StencilWriteMask = 0xFF;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilDisable].first = depth_disabled_stencil_disable_desc;
		D3D12_DEPTH_STENCIL_DESC d3d12_depth_disabled_stencil_disable_desc = {};
		d3d12_depth_disabled_stencil_disable_desc.DepthEnable = FALSE;
		d3d12_depth_disabled_stencil_disable_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3d12_depth_disabled_stencil_disable_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3d12_depth_disabled_stencil_disable_desc.StencilEnable = FALSE;
		d3d12_depth_disabled_stencil_disable_desc.StencilReadMask = 0xFF;
		d3d12_depth_disabled_stencil_disable_desc.StencilWriteMask = 0xFF;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilDisable].first = d3d12_depth_disabled_stencil_disable_desc;

		D3D11_DEPTH_STENCIL_DESC depth_disabled_stencil_read_sky_desc = {};
		depth_disabled_stencil_read_sky_desc.DepthEnable = FALSE;
		depth_disabled_stencil_read_sky_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_disabled_stencil_read_sky_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_disabled_stencil_read_sky_desc.StencilEnable = TRUE;
		depth_disabled_stencil_read_sky_desc.StencilReadMask = 0xFF;
		depth_disabled_stencil_read_sky_desc.StencilWriteMask = 0xFF;
		depth_disabled_stencil_read_sky_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_disabled_stencil_read_sky_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depth_disabled_stencil_read_sky_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_disabled_stencil_read_sky_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		depth_disabled_stencil_read_sky_desc.BackFace = depth_disabled_stencil_read_sky_desc.FrontFace;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].first = depth_disabled_stencil_read_sky_desc;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].second = StencilMaskEnum_SKY;
		D3D12_DEPTH_STENCIL_DESC d3d12_depth_disabled_stencil_read_sky_desc = d3d12_depth_disabled_stencil_disable_desc;
		d3d12_depth_disabled_stencil_read_sky_desc.StencilEnable = TRUE;
		d3d12_depth_disabled_stencil_read_sky_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3d12_depth_disabled_stencil_read_sky_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3d12_depth_disabled_stencil_read_sky_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3d12_depth_disabled_stencil_read_sky_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
		d3d12_depth_disabled_stencil_read_sky_desc.BackFace = d3d12_depth_disabled_stencil_read_sky_desc.FrontFace;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].first = d3d12_depth_disabled_stencil_read_sky_desc;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].second = StencilMaskEnum_SKY;

		D3D11_DEPTH_STENCIL_DESC depth_read_only_desc = {};
        depth_read_only_desc.DepthEnable = TRUE;
        depth_read_only_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_read_only_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		m_d3d11_state_cache->depth_stencil_state_create_desc[DepthStencilStateType_DepthReadOnly].first = depth_read_only_desc;
		D3D12_DEPTH_STENCIL_DESC d3d12_depth_read_only_desc = {};
		d3d12_depth_read_only_desc.DepthEnable = TRUE;
		d3d12_depth_read_only_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		d3d12_depth_read_only_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		d3d12_depth_read_only_desc.StencilEnable = FALSE;
		m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[DepthStencilStateType_DepthReadOnly].first = d3d12_depth_read_only_desc;

	}

	void DolasRHI::InitializeBlendStateCreateDesc()
	{
		// Opaque
		D3D11_BLEND_DESC opaque_desc = {};
        opaque_desc.AlphaToCoverageEnable = FALSE;
        opaque_desc.IndependentBlendEnable = FALSE;
        auto& opaque_rt = opaque_desc.RenderTarget[0];
        opaque_rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        opaque_rt.BlendEnable = FALSE;
        opaque_rt.SrcBlend = D3D11_BLEND_ONE;
        opaque_rt.DestBlend = D3D11_BLEND_ZERO;
        opaque_rt.BlendOp = D3D11_BLEND_OP_ADD;
        opaque_rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        opaque_rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        opaque_rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        m_d3d11_state_cache->blend_state_create_desc[BlendStateType_Opaque] = opaque_desc;
		D3D12_BLEND_DESC d3d12_opaque_desc = {};
		d3d12_opaque_desc.AlphaToCoverageEnable = FALSE;
		d3d12_opaque_desc.IndependentBlendEnable = FALSE;
		auto& d3d12_opaque_rt = d3d12_opaque_desc.RenderTarget[0];
		d3d12_opaque_rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		d3d12_opaque_rt.BlendEnable = FALSE;
		d3d12_opaque_rt.SrcBlend = D3D12_BLEND_ONE;
		d3d12_opaque_rt.DestBlend = D3D12_BLEND_ZERO;
		d3d12_opaque_rt.BlendOp = D3D12_BLEND_OP_ADD;
		d3d12_opaque_rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		d3d12_opaque_rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		d3d12_opaque_rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		m_d3d11_state_cache->d3d12_blend_state_create_desc[BlendStateType_Opaque] = d3d12_opaque_desc;

        // AlphaBlend
		D3D11_BLEND_DESC alpha_blend_desc = {};
        alpha_blend_desc.AlphaToCoverageEnable = FALSE;
        alpha_blend_desc.IndependentBlendEnable = FALSE;
        auto& alpha_blend_rt = alpha_blend_desc.RenderTarget[0];
		alpha_blend_rt.BlendEnable = TRUE;
        alpha_blend_rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        alpha_blend_rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        alpha_blend_rt.BlendOp = D3D11_BLEND_OP_ADD;
        alpha_blend_rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        alpha_blend_rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        alpha_blend_rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        m_d3d11_state_cache->blend_state_create_desc[BlendStateType_AlphaBlend] = alpha_blend_desc;
		D3D12_BLEND_DESC d3d12_alpha_blend_desc = d3d12_opaque_desc;
		auto& d3d12_alpha_blend_rt = d3d12_alpha_blend_desc.RenderTarget[0];
		d3d12_alpha_blend_rt.BlendEnable = TRUE;
		d3d12_alpha_blend_rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		d3d12_alpha_blend_rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		d3d12_alpha_blend_rt.BlendOp = D3D12_BLEND_OP_ADD;
		d3d12_alpha_blend_rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		d3d12_alpha_blend_rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		d3d12_alpha_blend_rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		m_d3d11_state_cache->d3d12_blend_state_create_desc[BlendStateType_AlphaBlend] = d3d12_alpha_blend_desc;

        // Additive
		D3D11_BLEND_DESC additive_desc = {};
        additive_desc.AlphaToCoverageEnable = FALSE;
        additive_desc.IndependentBlendEnable = FALSE;
        auto& additive_rt = additive_desc.RenderTarget[0];
        additive_rt.BlendEnable = TRUE;
        additive_rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        additive_rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        additive_rt.BlendOp = D3D11_BLEND_OP_ADD;
        additive_rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        additive_rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        additive_rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        m_d3d11_state_cache->blend_state_create_desc[BlendStateType_Additive] = additive_desc;
		D3D12_BLEND_DESC d3d12_additive_desc = d3d12_alpha_blend_desc;
		m_d3d11_state_cache->d3d12_blend_state_create_desc[BlendStateType_Additive] = d3d12_additive_desc;
	}

	void DolasRHI::InitializePrimitiveTopology()
	{
		m_d3d11_state_cache->primitive_topology[PrimitiveTopology_TriangleList] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_d3d11_state_cache->primitive_topology[PrimitiveTopology_TriangleStrip] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		m_d3d11_state_cache->d3d12_primitive_topology[PrimitiveTopology_TriangleList] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_d3d11_state_cache->d3d12_primitive_topology[PrimitiveTopology_TriangleStrip] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		m_d3d11_state_cache->d3d12_primitive_topology_type[PrimitiveTopology_TriangleList] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		m_d3d11_state_cache->d3d12_primitive_topology_type[PrimitiveTopology_TriangleStrip] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

	void DolasRHI::InitializeInputLayoutDescs()
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_desc = m_d3d11_state_cache->input_element_descs[InputLayoutType_POS_3];
		pos_3_desc =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_d3d11_state_cache->d3d12_input_element_descs[InputLayoutType_POS_3] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_uv_2_desc = m_d3d11_state_cache->input_element_descs[InputLayoutType_POS_3_UV_2];
		pos_3_uv_2_desc =
		{
			// POSITION 来自 slot 0，stride = 3 * 4 bytes，offset = 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TEXCOORD 来自 slot 1，单独一个 UV buffer，每个顶点从 offset = 0 开始
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_d3d11_state_cache->d3d12_input_element_descs[InputLayoutType_POS_3_UV_2] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_uv_2_norm_3_desc = m_d3d11_state_cache->input_element_descs[InputLayoutType_POS_3_UV_2_NORM_3];
		pos_3_uv_2_norm_3_desc =
		{
			// POSITION: slot 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TEXCOORD: slot 1
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// NORMAL:   slot 2
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_d3d11_state_cache->d3d12_input_element_descs[InputLayoutType_POS_3_UV_2_NORM_3] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_uv_2_norm_3_tang_3_desc = m_d3d11_state_cache->input_element_descs[InputLayoutType_POS_3_UV_2_NORM_3_TANG_3];
		pos_3_uv_2_norm_3_tang_3_desc =
		{
			// POSITION: slot 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TEXCOORD: slot 1
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// NORMAL:   slot 2
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TANGENT:  slot 3
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_d3d11_state_cache->d3d12_input_element_descs[InputLayoutType_POS_3_UV_2_NORM_3_TANG_3] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_norm_3_desc = m_d3d11_state_cache->input_element_descs[InputLayoutType_POS_3_NORM_3];
		pos_3_norm_3_desc =
		{
			// POSITION: slot 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// NORMAL:   slot 1
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_d3d11_state_cache->d3d12_input_element_descs[InputLayoutType_POS_3_NORM_3] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
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

	std::shared_ptr<RenderTargetView> DolasRHI::CreateRenderTargetView(TextureID texture_id)
	{
		TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
		DOLAS_RETURN_NULL_IF_NULL(texture_manager);

		Texture* texture = texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_NULL_IF_NULL(texture);

		ID3D11Texture2D* d3d11_texture = texture->GetD3DTexture2D();
		std::shared_ptr<RenderTargetView> render_target_view = nullptr;
		if (d3d11_texture)
		{
			render_target_view = CreateRenderTargetViewByD3D11Texture(d3d11_texture);
		}
		else
		{
			render_target_view = std::make_shared<RenderTargetView>();
		}
		render_target_view->m_texture_id = texture_id;
		render_target_view->m_d3d12_render_target_view = texture->GetD3D12RtvHandle();
		return render_target_view;
	}

	std::shared_ptr<RenderTargetView> DolasRHI::CreateRenderTargetViewByD3D11Texture(ID3D11Texture2D* d3d_texture)
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		d3d_texture->GetDesc(&texture_desc);

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

		std::shared_ptr<RenderTargetView> render_target_view = std::make_shared<RenderTargetView>();

		if (m_d3d_device)
		{
			HR(m_d3d_device->CreateRenderTargetView(d3d_texture, &rtv_desc, &render_target_view->m_d3d_render_target_view));
		}

		return render_target_view;
	}

	ID3D11RasterizerState* DolasRHI::CreateRasterizerState(RasterizerStateType type)
	{
		D3D11_RASTERIZER_DESC desc = m_d3d11_state_cache->rasterizer_state_create_desc[type];

		ID3D11RasterizerState* rasterizer_state = nullptr;
		HR(m_d3d_device->CreateRasterizerState(&desc, &rasterizer_state));

		return rasterizer_state;
	}

	Bool DolasRHI::CreateDepthStencilState(DepthStencilStateType type)
	{
		D3D11_DEPTH_STENCIL_DESC desc = m_d3d11_state_cache->depth_stencil_state_create_desc[type].first;
		UInt stencil_ref_value = m_d3d11_state_cache->depth_stencil_state_create_desc[type].second;

		ID3D11DepthStencilState* d3d_depth_stencil_state = nullptr;
		HR(m_d3d_device->CreateDepthStencilState(&desc, &d3d_depth_stencil_state));

		m_depth_stencil_states[type].m_d3d_depth_stencil_state = d3d_depth_stencil_state;
		m_depth_stencil_states[type].m_stencil_ref_value = stencil_ref_value;
		return true;
	}

	ID3D11BlendState* DolasRHI::CreateBlendState(BlendStateType type)
	{
		D3D11_BLEND_DESC desc = m_d3d11_state_cache->blend_state_create_desc[type];
		ID3D11BlendState* blend_state = nullptr;
		HR(m_d3d_device->CreateBlendState(&desc, &blend_state));

		return blend_state;
	}

	std::shared_ptr<InputLayout> DolasRHI::CreateInputLayout(InputLayoutType input_layout_type, const void* pShaderBytecodeWithInputSignature, std::size_t BytecodeLength)
	{
		std::shared_ptr<InputLayout> input_layout = std::make_shared<InputLayout>();
		ID3D11InputLayout* d3d11_input_layout = nullptr;
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc = m_d3d11_state_cache->input_element_descs[input_layout_type];
        std::size_t elem_count_sz = input_element_desc.size();
        if (elem_count_sz > (std::size_t)(std::numeric_limits<UINT>::max)()) elem_count_sz = (std::size_t)(std::numeric_limits<UINT>::max)();
		HR(m_d3d_device->CreateInputLayout(input_element_desc.data(), (UINT)elem_count_sz, pShaderBytecodeWithInputSignature, BytecodeLength, &d3d11_input_layout));
		input_layout->m_d3d_input_layout = d3d11_input_layout;
		return input_layout;
	}

	const DepthStencilState& DolasRHI::GetOrCreateDepthStencilState(DepthStencilStateType type)
	{
		if (!m_depth_stencil_states[type].initialized)
		{
			if (CreateDepthStencilState(type))
			{
				m_depth_stencil_states[type].initialized = true;
			}
		}
		
		return m_depth_stencil_states[type];
	}

	void DolasRHI::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before_state, D3D12_RESOURCE_STATES after_state)
	{
		if (!resource || before_state == after_state)
		{
			return;
		}

		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (!command_list)
		{
			return;
		}

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = before_state;
		barrier.Transition.StateAfter = after_state;
		command_list->ResourceBarrier(1, &barrier);
	}

	void DolasRHI::TransitionTexture(Texture* texture, D3D12_RESOURCE_STATES after_state)
	{
		if (!texture || !texture->GetD3D12Resource())
		{
			return;
		}

		D3D12_RESOURCE_STATES before_state = texture->GetD3D12ResourceState();
		TransitionResource(texture->GetD3D12Resource(), before_state, after_state);
		texture->SetD3D12ResourceState(after_state);
	}

	void DolasRHI::UpdateD3D12UploadBuffer(ID3D12Resource* resource, const void* data, std::size_t size)
	{
		if (!resource || !data || size == 0)
		{
			return;
		}

		D3D12_RANGE read_range = { 0, 0 };
		void* mapped_data = nullptr;
		HRESULT hr = resource->Map(0, &read_range, &mapped_data);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to map D3D12 constant buffer, HRESULT: 0x{0:X}", hr);
			return;
		}

		memcpy(mapped_data, data, size);
		D3D12_RANGE written_range = { 0, size };
		resource->Unmap(0, &written_range);
	}

	void DolasRHI::BindD3D12GlobalResources()
	{
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (!command_list || !m_d3d12_root_signature)
		{
			return;
		}

		command_list->SetGraphicsRootSignature(m_d3d12_root_signature);
		if (m_d3d12_per_view_parameters_buffer)
		{
			command_list->SetGraphicsRootConstantBufferView(kRootPerViewCBV, m_d3d12_per_view_parameters_buffer->GetGPUVirtualAddress());
		}
		if (m_d3d12_per_frame_parameters_buffer)
		{
			command_list->SetGraphicsRootConstantBufferView(kRootPerFrameCBV, m_d3d12_per_frame_parameters_buffer->GetGPUVirtualAddress());
		}
		if (m_d3d12_per_object_parameters_buffer)
		{
			command_list->SetGraphicsRootConstantBufferView(kRootPerObjectCBV, m_d3d12_per_object_parameters_buffer->GetGPUVirtualAddress());
		}
		if (m_d3d12_dummy_constant_buffer)
		{
			command_list->SetGraphicsRootConstantBufferView(kRootVSGlobalCBV, m_d3d12_dummy_constant_buffer->GetGPUVirtualAddress());
			command_list->SetGraphicsRootConstantBufferView(kRootPSGlobalCBV, m_d3d12_dummy_constant_buffer->GetGPUVirtualAddress());
		}
	}

	void DolasRHI::BindD3D12SrvTable(std::shared_ptr<ShaderContext> shader_context, bool pixel_shader)
	{
		DOLAS_RETURN_IF_NULL(shader_context);
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12Device* device = rhi ? rhi->GetDevice() : nullptr;
		ID3D12GraphicsCommandList* command_list = rhi ? rhi->GetCommandList() : nullptr;
		if (!rhi || !device || !command_list)
		{
			return;
		}

		for (const auto& texture_pair : shader_context->GetSlotToTextureMap())
		{
			Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_pair.second);
			DOLAS_CONTINUE_IF_NULL(texture);
			TransitionTexture(texture, pixel_shader ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE table_cpu = {};
		D3D12_GPU_DESCRIPTOR_HANDLE table_gpu = {};
		if (!rhi->AllocateTransientSrvDescriptorTable(kD3D12SrvTableSize, &table_cpu, &table_gpu))
		{
			return;
		}

		const UINT descriptor_size = rhi->GetSrvDescriptorSize();
		D3D12_CPU_DESCRIPTOR_HANDLE null_srv = rhi->GetNullSrvDescriptorCpuHandle();
		for (UINT slot = 0; slot < kD3D12SrvTableSize; ++slot)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE dst = table_cpu;
			dst.ptr += static_cast<SIZE_T>(slot) * descriptor_size;
			device->CopyDescriptorsSimple(1, dst, null_srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		for (const auto& srv_pair : shader_context->GetSlotToD3D12SRVCpuMap())
		{
			if (srv_pair.first >= kD3D12SrvTableSize || srv_pair.second.ptr == 0)
			{
				continue;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE dst = table_cpu;
			dst.ptr += static_cast<SIZE_T>(srv_pair.first) * descriptor_size;
			device->CopyDescriptorsSimple(1, dst, srv_pair.second, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		command_list->SetGraphicsRootDescriptorTable(pixel_shader ? kRootPSSrvTable : kRootVSSrvTable, table_gpu);
	}

	ID3D12PipelineState* DolasRHI::GetOrCreateD3D12PipelineState(RenderPrimitive* render_primitive)
	{
		DOLAS_RETURN_NULL_IF_NULL(render_primitive);
		RenderHardwareInterface* rhi = g_dolas_engine.m_render_hardware_interface;
		ID3D12Device* device = rhi ? rhi->GetDevice() : nullptr;
		if (!device || !m_d3d12_root_signature || !m_current_vertex_context || !m_current_pixel_context)
		{
			return nullptr;
		}

		ShaderBytecodeView vs_bytecode = m_current_vertex_context->GetShaderBytecode();
		ShaderBytecodeView ps_bytecode = m_current_pixel_context->GetShaderBytecode();
		if (!vs_bytecode.IsValid() || !ps_bytecode.IsValid())
		{
			return nullptr;
		}

		std::size_t key = 0;
		key = HashCombine(key, reinterpret_cast<std::size_t>(vs_bytecode.data));
		key = HashCombine(key, vs_bytecode.size);
		key = HashCombine(key, reinterpret_cast<std::size_t>(ps_bytecode.data));
		key = HashCombine(key, ps_bytecode.size);
		key = HashCombine(key, static_cast<std::size_t>(render_primitive->m_input_layout_type));
		key = HashCombine(key, static_cast<std::size_t>(m_current_rasterizer_state_type));
		key = HashCombine(key, static_cast<std::size_t>(m_current_depth_stencil_state_type));
		key = HashCombine(key, static_cast<std::size_t>(m_current_blend_state_type));
		key = HashCombine(key, static_cast<std::size_t>(m_current_primitive_topology));
		key = HashCombine(key, static_cast<std::size_t>(m_current_render_target_count));
		key = HashCombine(key, static_cast<std::size_t>(m_current_dsv_format));
		for (UINT i = 0; i < m_current_render_target_count; ++i)
		{
			key = HashCombine(key, static_cast<std::size_t>(m_current_rtv_formats[i]));
		}

		auto pso_iter = m_d3d12_pipeline_state_cache.find(key);
		if (pso_iter != m_d3d12_pipeline_state_cache.end())
		{
			return pso_iter->second;
		}

		const std::vector<D3D12_INPUT_ELEMENT_DESC>& input_descs =
			m_d3d11_state_cache->d3d12_input_element_descs[render_primitive->m_input_layout_type];

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
		pso_desc.InputLayout = { input_descs.data(), static_cast<UINT>(input_descs.size()) };
		pso_desc.pRootSignature = m_d3d12_root_signature;
		pso_desc.VS = { vs_bytecode.data, vs_bytecode.size };
		pso_desc.PS = { ps_bytecode.data, ps_bytecode.size };
		pso_desc.RasterizerState = m_d3d11_state_cache->d3d12_rasterizer_state_create_desc[m_current_rasterizer_state_type];
		pso_desc.BlendState = m_d3d11_state_cache->d3d12_blend_state_create_desc[m_current_blend_state_type];
		pso_desc.DepthStencilState = m_d3d11_state_cache->d3d12_depth_stencil_state_create_desc[m_current_depth_stencil_state_type].first;
		pso_desc.SampleMask = UINT_MAX;
		pso_desc.PrimitiveTopologyType = m_d3d11_state_cache->d3d12_primitive_topology_type[m_current_primitive_topology];
		pso_desc.NumRenderTargets = m_current_render_target_count;
		for (UINT i = 0; i < m_current_render_target_count; ++i)
		{
			pso_desc.RTVFormats[i] = m_current_rtv_formats[i];
		}
		pso_desc.DSVFormat = m_current_dsv_format;
		pso_desc.SampleDesc.Count = 1;
		pso_desc.SampleDesc.Quality = 0;

		ID3D12PipelineState* pipeline_state = nullptr;
		HRESULT hr = device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state));
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create D3D12 graphics pipeline state! HRESULT: 0x{0:X}", hr);
			return nullptr;
		}

		m_d3d12_pipeline_state_cache[key] = pipeline_state;
		return pipeline_state;
	}

	void DolasRHI::RenderImGuiDrawData()
	{
		g_dolas_engine.m_imgui_manager->RenderDrawData(g_dolas_engine.m_render_hardware_interface->GetCommandList());
	}

} // namespace Dolas

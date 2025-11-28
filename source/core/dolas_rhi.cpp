#include "dolas_rhi.h"
#include "dolas_engine.h"
#include "dxgi_helper.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_input_manager.h"
#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For D3D11 debug interfaces
#endif

#include <iostream>
#include <imm.h>  // 输入法控制（切换到英文输入状态）
#include "render/dolas_render_camera.h"
#include "manager/dolas_log_system_manager.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_shader.h"
namespace Dolas
{
    LRESULT CALLBACK
    MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Forward hwnd on because we can get messages (e.g., WM_CREATE)
        // before CreateWindow returns, and thus before m_hMainWnd is valid.
        return g_dolas_engine.m_input_manager->MsgProc(hwnd, msg, wParam, lParam);
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
		: m_d3d_viewport(D3D11_VIEWPORT())
	{
		m_d3d_viewport.TopLeftX = top_left_x;
		m_d3d_viewport.TopLeftY = top_left_y;
		m_d3d_viewport.Width = width;
		m_d3d_viewport.Height = height;
		m_d3d_viewport.MinDepth = min_depth;
		m_d3d_viewport.MaxDepth = max_depth;
	}
	ViewPort::~ViewPort()
	{
		m_d3d_viewport = D3D11_VIEWPORT();
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
			LOG_ERROR("Failed to create per view constant buffer!");
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
			LOG_ERROR("Failed to create per frame constant buffer!");
			return false;
		}

		// Per object
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.ByteWidth = sizeof(PerObjectConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		// 新建常量缓冲区，不使用初始数据
		hr = m_d3d_device->CreateBuffer(&cbd, nullptr, &m_d3d_per_object_parameters_buffer);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create per frame constant buffer!");
			return false;
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
	}

	void DolasRHI::Present(TextureID scene_result_texture_id)
	{
		Texture* scene_result_texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(scene_result_texture_id);
		DOLAS_RETURN_IF_NULL(scene_result_texture);
		m_d3d_immediate_context->CopyResource(m_swap_chain_back_texture, scene_result_texture->GetD3DTexture2D());
		m_swap_chain->Present(0, 0);
	}
    
	void DolasRHI::SetRenderTargetViewAndDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view, std::shared_ptr<DepthStencilView> depth_stencil_view)
	{
		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
        if (d3d11_render_target_view.size() == 0) {
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, depth_stencil_view->m_d3d_depth_stencil_view);
            return;
        }

        ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
            d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i]->m_d3d_render_target_view : nullptr;
		}
        m_d3d_immediate_context->OMSetRenderTargets(d3d11_render_target_view.size(), d3d11_render_target_view_array, depth_stencil_view->m_d3d_depth_stencil_view);
	}

    void DolasRHI::SetRenderTargetViewWithoutDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view)
    {
		const UInt k_max_render_targets = 10; // D3D11允许最多10个渲染目标
		if (d3d11_render_target_view.size() == 0) {
			return;
		}

		ID3D11RenderTargetView* d3d11_render_target_view_array[k_max_render_targets] = { nullptr };
		for (UInt i = 0; i < k_max_render_targets; i++)
		{
			d3d11_render_target_view_array[i] = i < d3d11_render_target_view.size() ? d3d11_render_target_view[i]->m_d3d_render_target_view : nullptr;
		}
		m_d3d_immediate_context->OMSetRenderTargets(d3d11_render_target_view.size(), d3d11_render_target_view_array, nullptr);
    }

	void DolasRHI::ClearRenderTargetView(std::shared_ptr<RenderTargetView> rtv, const Float clear_color[4])
	{
		m_d3d_immediate_context->ClearRenderTargetView(rtv->m_d3d_render_target_view, clear_color);
	}

	std::shared_ptr<RenderTargetView> DolasRHI::GetBackBufferRTV() const { return m_back_buffer_render_target_view; }

	std::shared_ptr<DepthStencilView> DolasRHI::CreateDepthStencilView(TextureID texture_id)
	{
		TextureManager* texture_manager = g_dolas_engine.m_texture_manager;
		DOLAS_RETURN_NULL_IF_NULL(texture_manager);

		Texture* texture = texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_NULL_IF_NULL(texture);

		ID3D11Texture2D* d3d_texture_2d = texture->GetD3DTexture2D();
		DOLAS_RETURN_NULL_IF_NULL(d3d_texture_2d);

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

		std::shared_ptr<DepthStencilView> depth_stencil_view = std::make_shared<DepthStencilView>();
		HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateDepthStencilView(d3d_texture_2d, &dsv_desc, &depth_stencil_view->m_d3d_depth_stencil_view);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create depth stencil view!");
			return nullptr;
		}

		return depth_stencil_view;
	}

	void DolasRHI::ClearDepthStencilView(std::shared_ptr<DepthStencilView> dsv, const DepthClearParams& depth_clear_params, const StencilClearParams& stencil_clear_params)
	{
		UINT clear_flags = 0;
		if (depth_clear_params.enable)
		{
			clear_flags |= D3D11_CLEAR_DEPTH;
		}
		if (stencil_clear_params.enable)
		{
			clear_flags |= D3D11_CLEAR_STENCIL;
		}

		m_d3d_immediate_context->ClearDepthStencilView(dsv->m_d3d_depth_stencil_view, clear_flags, depth_clear_params.clear_value, stencil_clear_params.clear_value);
	}

    void DolasRHI::SetViewPort(const ViewPort& viewport)
	{
		m_d3d_immediate_context->RSSetViewports(1, &viewport.m_d3d_viewport);
	}
	
	void DolasRHI::SetRasterizerState(RasterizerStateType type)
	{
		RasterizerState& rasterizer_state = m_rasterizer_states[static_cast<UInt>(type)];
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
		const DepthStencilState& depth_stencil_state = GetOrCreateDepthStencilState(type);
        m_d3d_immediate_context->OMSetDepthStencilState(depth_stencil_state.m_d3d_depth_stencil_state, depth_stencil_state.m_stencil_ref_value);
    }

    void DolasRHI::SetBlendState(BlendStateType type)
    {
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

	Bool DolasRHI::BindVertexContext(VertexContext* vertex_context, const std::unordered_map<int, TextureID>& materal_textures_map, ID3D11ClassInstance* const* class_instances/* = nullptr*/, UINT num_class_instances/* = 0*/)
	{
		m_d3d_immediate_context->VSSetShader(vertex_context->GetD3DVertexShader(), class_instances, num_class_instances);

		for (auto tex_slot_id_pair : materal_textures_map)
		{
			int slot = tex_slot_id_pair.first;
			TextureID texture_id = tex_slot_id_pair.second;
			Texture* tex = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
			DOLAS_CONTINUE_IF_NULL(tex);
			ID3D11ShaderResourceView* srv = tex->GetShaderResourceView();
			DOLAS_CONTINUE_IF_NULL(srv);
			vertex_context->SetShaderResourceView(slot, srv);
		}

		const std::unordered_map<size_t, ID3D11ShaderResourceView*>& srvs = vertex_context->GetShaderResourceViews();
		for (auto srv_iter : srvs)
		{
			m_d3d_immediate_context->VSSetShaderResources(srv_iter.first, 1, &(srv_iter.second));
		}

		m_current_vs_blob = vertex_context->GetD3DShaderBlob();

		return true;
	}

	// PixelContext
	Bool DolasRHI::BindPixelContext(PixelContext* pixel_context, const std::unordered_map<int, TextureID>& materal_textures_map, ID3D11ClassInstance* const* class_instances/* = nullptr*/, UINT num_class_instances/* = 0*/)
	{
		m_d3d_immediate_context->PSSetShader(pixel_context->GetD3DPixelShader(), class_instances, num_class_instances);

		for (auto tex_slot_id_pair : materal_textures_map)
		{
			int slot = tex_slot_id_pair.first;
			TextureID texture_id = tex_slot_id_pair.second;
			Texture* tex = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
			DOLAS_CONTINUE_IF_NULL(tex);
			ID3D11ShaderResourceView* srv = tex->GetShaderResourceView();
			DOLAS_CONTINUE_IF_NULL(srv);
			pixel_context->SetShaderResourceView(slot, srv);
		}
		const std::unordered_map<size_t, ID3D11ShaderResourceView*>& srvs = pixel_context->GetShaderResourceViews();
		for (auto srv_iter : srvs)
		{
			m_d3d_immediate_context->PSSetShaderResources(srv_iter.first, 1, &(srv_iter.second));
		}

		return true;
	}

	void DolasRHI::SetPrimitiveTopology(PrimitiveTopology primitive_topology)
	{
		m_d3d_immediate_context->IASetPrimitiveTopology(m_d3d11_primitive_topology[static_cast<UInt>(primitive_topology)]);
	}

	void DolasRHI::SetInputLayout(InputLayoutType input_layout_type, const void* vs_blob, size_t bytecode_length)
	{
		std::shared_ptr<InputLayout> input_layout = CreateInputLayout(input_layout_type, vs_blob, bytecode_length);
		m_d3d_immediate_context->IASetInputLayout(input_layout->m_d3d_input_layout);
	}

	void DolasRHI::SetVertexBuffers(const std::vector<BufferID>& vertex_buffer_ids, const std::vector<UInt>& vertex_strides, const std::vector<UInt>& vertex_offsets)
	{
		std::vector<ID3D11Buffer*> d3d11_buffers;
		for (int i = 0; i < vertex_buffer_ids.size(); i++)
		{
			Buffer* buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(vertex_buffer_ids[i]);
			d3d11_buffers.push_back(buffer->GetBuffer());
		}
		m_d3d_immediate_context->IASetVertexBuffers(0, d3d11_buffers.size(), d3d11_buffers.data(), vertex_strides.data(), vertex_offsets.data());
	}

	void DolasRHI::SetIndexBuffer(BufferID index_buffer_id)
	{
		Buffer* buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(index_buffer_id);
		DOLAS_RETURN_IF_NULL(buffer);
		m_d3d_immediate_context->IASetIndexBuffer(buffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	void DolasRHI::DrawIndexed(UInt index_count)
	{
		m_d3d_immediate_context->DrawIndexed(index_count, 0, 0);
	}

	void DolasRHI::DrawRenderPrimitive(RenderPrimitiveID render_primitive_id)
	{
		RenderPrimitive* render_primitive = g_dolas_engine.m_render_primitive_manager->GetRenderPrimitiveByID(render_primitive_id);
		DOLAS_RETURN_IF_NULL(render_primitive);

		SetInputLayout(render_primitive->m_input_layout_type, m_current_vs_blob->GetBufferPointer(), m_current_vs_blob->GetBufferSize());

		SetPrimitiveTopology(render_primitive->m_topology);

		SetVertexBuffers(render_primitive->m_vertex_buffer_ids, render_primitive->m_vertex_strides, render_primitive->m_vertex_offsets);

		SetIndexBuffer(render_primitive->m_index_buffer_id);

		DrawIndexed(render_primitive->m_index_count);
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
		m_d3d_immediate_context->VSSetConstantBuffers(2, 1, &m_d3d_per_object_parameters_buffer);
	}

	void DolasRHI::PSSetConstantBuffers()
	{
		m_d3d_immediate_context->PSSetConstantBuffers(0, 1, &m_d3d_per_view_parameters_buffer);
		m_d3d_immediate_context->PSSetConstantBuffers(1, 1, &m_d3d_per_frame_parameters_buffer);
		m_d3d_immediate_context->PSSetConstantBuffers(2, 1, &m_d3d_per_object_parameters_buffer);
	}

	ID3D11ShaderResourceView* DolasRHI::CreateShaderResourceView(ID3D11Resource* resource)
	{
		ID3D11ShaderResourceView* shader_resource_view = nullptr;
		HRESULT hr = m_d3d_device->CreateShaderResourceView(resource, nullptr, &shader_resource_view);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create shader resource view!");
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
			LOG_ERROR("Failed to map per view constant buffer!");
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
			LOG_ERROR("Failed to map per view constant buffer!");
			return;
		}
		memcpy_s(mappedData.pData, sizeof(per_view_constant_buffer), &per_view_constant_buffer, sizeof(per_view_constant_buffer));
		m_d3d_immediate_context->Unmap(m_d3d_per_view_parameters_buffer, 0);
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

		per_object_constant_buffer.world = trans_mat * scale_mat * ratation_mat;

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = m_d3d_immediate_context->Map(m_d3d_per_object_parameters_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to map per object constant buffer!");
			return;
		}
		memcpy_s(mappedData.pData, sizeof(per_object_constant_buffer), &per_object_constant_buffer, sizeof(per_object_constant_buffer));
		m_d3d_immediate_context->Unmap(m_d3d_per_object_parameters_buffer, 0);
	}

	Bool DolasRHI::InitializeWindow()
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

        // 启动时彻底禁用这个窗口上的 IME（直接输入，不经过中文输入法）
        // 这样键盘消息不会被输入法拦截，可直接用于游戏按键。
        ImmAssociateContext(m_window_handle, NULL);

        return true;
	}

	bool DolasRHI::InitializeD3D()
	{
        IDXGIAdapter* best_adapter = DXGIHelper::SelectBestAdapter();
		if (!best_adapter) {
			LOG_ERROR("Failed to select best adapter!");
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
		// Set to 0/0 to let DXGI choose the best refresh rate (no limit)
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 0;
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

		m_back_buffer_render_target_view = CreateRenderTargetViewByD3D11Texture(m_swap_chain_back_texture);

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

		InitializeRasterizerStateCreateDesc();
		InitializeDepthStencilStateCreateDesc();
		InitializeBlendStateCreateDesc();
		InitializePrimitiveTopology();
		InitializeInputLayoutDescs();
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
		m_rasterizer_state_create_desc[RasterizerStateType_SolidNoneCull] = solid_none_cull_desc;

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
		m_rasterizer_state_create_desc[RasterizerStateType_SolidBackCull] = solid_back_cull_desc;

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
		m_rasterizer_state_create_desc[RasterizerStateType_SolidFrontCull] = solid_front_cull_desc;

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
		m_rasterizer_state_create_desc[RasterizerStateType_Wireframe] = wireframe_desc;
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
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].first = depth_write_less_stencil_read_static_desc;
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthWriteLess_StencilWriteStatic].second = StencilMaskEnum_Static;

		D3D11_DEPTH_STENCIL_DESC depth_disabled_stencil_disable_desc = {};
		depth_disabled_stencil_disable_desc.DepthEnable = FALSE;
		depth_disabled_stencil_disable_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_disabled_stencil_disable_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_disabled_stencil_disable_desc.StencilEnable = FALSE;
		depth_disabled_stencil_disable_desc.StencilReadMask = 0xFF;
		depth_disabled_stencil_disable_desc.StencilWriteMask = 0xFF;
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilDisable].first = depth_disabled_stencil_disable_desc;

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
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].first = depth_disabled_stencil_read_sky_desc;
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthDisabled_StencilReadSky].second = StencilMaskEnum_SKY;

		D3D11_DEPTH_STENCIL_DESC depth_read_only_desc = {};
        depth_read_only_desc.DepthEnable = TRUE;
        depth_read_only_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_read_only_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		m_depth_stencil_state_create_desc[DepthStencilStateType_DepthReadOnly].first = depth_read_only_desc;

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
        m_blend_state_create_desc[BlendStateType_Opaque] = opaque_desc;

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
        m_blend_state_create_desc[BlendStateType_AlphaBlend] = alpha_blend_desc;

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
        m_blend_state_create_desc[BlendStateType_Additive] = additive_desc;
	}

	void DolasRHI::InitializePrimitiveTopology()
	{
		m_d3d11_primitive_topology[PrimitiveTopology_TriangleList] = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	void DolasRHI::InitializeInputLayoutDescs()
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_desc = m_input_element_descs[InputLayoutType_POS_3];
		pos_3_desc =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_uv_2_desc = m_input_element_descs[InputLayoutType_POS_3_UV_2];
		pos_3_uv_2_desc =
		{
			// POSITION 来自 slot 0，stride = 3 * 4 bytes，offset = 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TEXCOORD 来自 slot 1，单独一个 UV buffer，每个顶点从 offset = 0 开始
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		std::vector<D3D11_INPUT_ELEMENT_DESC>& pos_3_uv_2_norm_3_desc = m_input_element_descs[InputLayoutType_POS_3_UV_2_NORM_3];
		pos_3_uv_2_norm_3_desc =
		{
			// POSITION: slot 0
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// TEXCOORD: slot 1
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// NORMAL:   slot 2
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
		return CreateRenderTargetViewByD3D11Texture(d3d11_texture);
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

		HRESULT hr = m_d3d_device->CreateRenderTargetView(d3d_texture, &rtv_desc, &render_target_view->m_d3d_render_target_view);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create render target view!");
			return nullptr;
		}

		return render_target_view;
	}

	ID3D11RasterizerState* DolasRHI::CreateRasterizerState(RasterizerStateType type)
	{
		D3D11_RASTERIZER_DESC desc = m_rasterizer_state_create_desc[type];

		ID3D11RasterizerState* rasterizer_state = nullptr;
		HRESULT hr = m_d3d_device->CreateRasterizerState(&desc, &rasterizer_state);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create rasterizer state!");
			return nullptr;
		}
		return rasterizer_state;
	}

	Bool DolasRHI::CreateDepthStencilState(DepthStencilStateType type)
	{
		D3D11_DEPTH_STENCIL_DESC desc = m_depth_stencil_state_create_desc[type].first;
		UInt stencil_ref_value = m_depth_stencil_state_create_desc[type].second;

		ID3D11DepthStencilState* d3d_depth_stencil_state = nullptr;
		HRESULT hr = m_d3d_device->CreateDepthStencilState(&desc, &d3d_depth_stencil_state);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create depth stencil state!");
			return false;
		}

		m_depth_stencil_states[type].m_d3d_depth_stencil_state = d3d_depth_stencil_state;
		m_depth_stencil_states[type].m_stencil_ref_value = stencil_ref_value;
		return true;
	}

	ID3D11BlendState* DolasRHI::CreateBlendState(BlendStateType type)
	{
		D3D11_BLEND_DESC desc = m_blend_state_create_desc[type];
		ID3D11BlendState* blend_state = nullptr;
		HRESULT hr = m_d3d_device->CreateBlendState(&desc, &blend_state);
		if (FAILED(hr))
		{
			LOG_ERROR("Failed to create blend state!");
			return nullptr;
		}
		return blend_state;
	}

	std::shared_ptr<InputLayout> DolasRHI::CreateInputLayout(InputLayoutType input_layout_type, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength)
	{
		std::shared_ptr<InputLayout> input_layout = std::make_shared<InputLayout>();
		ID3D11InputLayout* d3d11_input_layout = nullptr;
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& input_element_desc = m_input_element_descs[input_layout_type];
		m_d3d_device->CreateInputLayout(input_element_desc.data(), input_element_desc.size(), pShaderBytecodeWithInputSignature, BytecodeLength, &d3d11_input_layout);
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

} // namespace Dolas

#ifndef DOLAS_RHI_H
#define DOLAS_RHI_H

#include <d3d11.h>
#include <d3d11_1.h>
#include <Windows.h>
#include <vector>
#include <memory>
#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For ID3D11Debug and ID3D11InfoQueue
#endif

#include "common/dolas_hash.h"
#include "core/dolas_math.h"
namespace Dolas
{
#ifndef DEFAULT_CLIENT_WIDTH
#define DEFAULT_CLIENT_WIDTH 1920
#endif

#ifndef DEFAULT_CLIENT_HEIGHT
#define DEFAULT_CLIENT_HEIGHT 1080
#endif
	class RenderTargetView
	{
	public:
		RenderTargetView();
		~RenderTargetView();

		ID3D11RenderTargetView* m_d3d_render_target_view;
	};

	class DepthStencilView
	{
	public:
		DepthStencilView();
		~DepthStencilView();

		ID3D11DepthStencilView* m_d3d_depth_stencil_view;
	};

	class ViewPort
	{
	public:
		ViewPort(Float top_left_x, Float top_left_y, Float width, Float height, Float min_depth, Float max_depth);
		~ViewPort();
		D3D11_VIEWPORT m_d3d_viewport;
	};

	enum class RasterizerStateType : UInt
    {
		SolidNoneCull,
        SolidBackCull,
        SolidFrontCull,
        Wireframe,
		RasterizerStateCount,
    };

	class RasterizerState
	{
	public:
		RasterizerState();
		~RasterizerState();

		ID3D11RasterizerState* m_d3d_rasterizer_state;
	};

	enum class DepthStencilStateType : UInt
	{
		DepthEnabled,
		DepthDisabled,
		DepthReadOnly,
		DepthStencilStateCount,
	};

	class DepthStencilState
	{
	public:
		DepthStencilState();
		~DepthStencilState();

		ID3D11DepthStencilState* m_d3d_depth_stencil_state;
	};

	enum class BlendStateType : UInt
    {
        Opaque,
        AlphaBlend,
        Additive,
		BlendStateCount,
    };

	class BlendState
	{
	public:
		BlendState();
		~BlendState();

		ID3D11BlendState* m_d3d_blend_state;
	};

	struct PerViewConstantBuffer
	{
		Matrix4x4 view;
		Matrix4x4 proj;
		Vector4 camera_position; // w is unused
	};

	struct PerObjectConstantBuffer
	{
		Matrix4x4 world;
	};

	struct PerFrameConstantBuffer
	{
		Vector4 light_direction_intensity;
		Vector4 light_color; // w is unused
	};

	struct DepthClearParams
	{
		Bool enable = true;
		Float clear_value = 1.0f;
	};

	struct StencilClearParams
	{
		Bool enable = true;
		UInt clear_value = 0;
	};

	enum class PrimitiveTopology : UInt
	{
		PrimitiveTopology_TriangleList,
		PrimitiveTopology_Count
	};

	enum class InputLayoutType : UInt
	{
		InputLayoutType_POS_3,
		InputLayoutType_POS_3_UV_2,
		InputLayoutType_POS_3_UV_2_NORM_3,
		InputLayoutType_Count
	};

	class InputLayout
	{
	public:
		InputLayout();
		~InputLayout();

		ID3D11InputLayout* m_d3d_input_layout;
	};

	// 渲染硬件接口(RHI)相关定义将在这里
	class DolasRHI
	{
	public:
		DolasRHI();
		~DolasRHI();
		bool Initialize();
		void Clear();
		void Present(TextureID scene_result_texture_id);

		HWND GetWindowHandle() const { return m_window_handle; }
		ID3D11Device* GetD3D11Device() const { return m_d3d_device; }
		ID3D11DeviceContext* GetD3D11DeviceContext() const { return m_d3d_immediate_context; }

		void SetVertexShader();
		void SetPixelShader();

		void VSSetConstantBuffers();
		void PSSetConstantBuffers();
		
		ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Resource* resource);
		void UpdatePerFrameParameters();
		void UpdatePerViewParameters(class RenderCamera* render_camera);
		void UpdatePerObjectParameters(Pose pose);
		// User annotation helpers (RenderDoc / PIX markers)
		void BeginEvent(const wchar_t* name);
		void EndEvent();
		void SetMarker(const wchar_t* name);

		// RenderTargetView
		std::shared_ptr<RenderTargetView> CreateRenderTargetView(TextureID texture_id);
		void SetRenderTargetViewAndDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view, std::shared_ptr<DepthStencilView> depth_stencil_view);
		void SetRenderTargetViewWithoutDepthStencilView(const std::vector<std::shared_ptr<RenderTargetView>>& d3d11_render_target_view);
		void ClearRenderTargetView(std::shared_ptr<RenderTargetView> rtv, const Float clear_color[4]);
		std::shared_ptr<RenderTargetView> GetBackBufferRTV() const;

		// DepthStencilView
		std::shared_ptr<DepthStencilView> CreateDepthStencilView(TextureID texture_id);
		void ClearDepthStencilView(std::shared_ptr<DepthStencilView> dsv, const DepthClearParams& depth_clear_params, const StencilClearParams& stencil_clear_params);
	
		// ViewPort
		void SetViewPort(const ViewPort& viewport);

		// RasterizerState
		void SetRasterizerState(RasterizerStateType type);

		// DepthStencilState
		void SetDepthStencilState(DepthStencilStateType type);

		// BlendState
		void SetBlendState(BlendStateType type);

		// PrimitiveTopology
		void SetPrimitiveTopology(PrimitiveTopology primitive_topology);
	private:
		bool InitializeWindow();
		bool InitializeD3D();

		void InitializeRasterizerStateCreateDesc();
		void InitializeDepthStencilStateCreateDesc();
		void InitializeBlendStateCreateDesc();
		void InitializePrimitiveTopology();
		void InitializeInputLayout();

		std::shared_ptr<RenderTargetView> CreateRenderTargetViewByD3D11Texture(ID3D11Texture2D* texture_id);
		ID3D11RasterizerState* CreateRasterizerState(RasterizerStateType type);
		ID3D11DepthStencilState* CreateDepthStencilState(DepthStencilStateType type);
		ID3D11BlendState* CreateBlendState(BlendStateType type);
		std::shared_ptr<InputLayout> CreateInputLayout(InputLayoutType input_layout_type, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength);

		ID3D11Device* m_d3d_device;
		ID3D11DeviceContext* m_d3d_immediate_context;

		HWND m_window_handle;
		int m_client_width;
		int m_client_height;

		ID3D11Buffer* m_d3d_per_frame_parameters_buffer;
		ID3D11Buffer* m_d3d_per_view_parameters_buffer;
		ID3D11Buffer* m_d3d_per_object_parameters_buffer;

		IDXGISwapChain* m_swap_chain;
		ID3D11Texture2D* m_swap_chain_back_texture;
		ID3DUserDefinedAnnotation* m_d3d_user_annotation;

		std::shared_ptr<RenderTargetView> m_back_buffer_render_target_view;
		RasterizerState m_rasterizer_states[static_cast<UInt>(RasterizerStateType::RasterizerStateCount)];
		DepthStencilState m_depth_stencil_states[static_cast<UInt>(DepthStencilStateType::DepthStencilStateCount)];
		BlendState m_blend_states[static_cast<UInt>(BlendStateType::BlendStateCount)];

		D3D11_RASTERIZER_DESC m_rasterizer_state_create_desc[static_cast<UInt>(RasterizerStateType::RasterizerStateCount)];
		D3D11_DEPTH_STENCIL_DESC m_depth_stencil_state_create_desc[static_cast<UInt>(DepthStencilStateType::DepthStencilStateCount)];
		D3D11_BLEND_DESC m_blend_state_create_desc[static_cast<UInt>(BlendStateType::BlendStateCount)];

		D3D11_PRIMITIVE_TOPOLOGY m_d3d11_primitive_topology[static_cast<UInt>(PrimitiveTopology::PrimitiveTopology_Count)];
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_input_element_descs[static_cast<UInt>(InputLayoutType::InputLayoutType_Count)];
	};

	// RAII scope for GPU events
	class UserAnnotationScope
	{
	public:
		UserAnnotationScope(DolasRHI* rhi, const wchar_t* name)
			: m_rhi(rhi)
		{
			if (m_rhi) m_rhi->BeginEvent(name);
		}

		~UserAnnotationScope()
		{
			if (m_rhi) m_rhi->EndEvent();
		}

	private:
		DolasRHI* m_rhi;
	};
}

#endif // DOLAS_RHI_H

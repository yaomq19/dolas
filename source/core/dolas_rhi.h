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
#include "core/dolas_rhi_common.h"
namespace Dolas
{
#ifndef DEFAULT_CLIENT_WIDTH
#define DEFAULT_CLIENT_WIDTH 1920
#endif

#ifndef DEFAULT_CLIENT_HEIGHT
#define DEFAULT_CLIENT_HEIGHT 1080
#endif
	

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
		
		// Buffer

		// Texture

		// DC
		void DrawRenderPrimitive(RenderPrimitiveID render_primitive_id, ID3DBlob* vs_blob);
	private:
		bool InitializeWindow();
		bool InitializeD3D();

		void InitializeRasterizerStateCreateDesc();
		void InitializeDepthStencilStateCreateDesc();
		void InitializeBlendStateCreateDesc();
		void InitializePrimitiveTopology();
		void InitializeInputLayoutDescs();

		std::shared_ptr<RenderTargetView> CreateRenderTargetViewByD3D11Texture(ID3D11Texture2D* texture_id);
		ID3D11RasterizerState* CreateRasterizerState(RasterizerStateType type);
		Bool CreateDepthStencilState(DepthStencilStateType type);
		ID3D11BlendState* CreateBlendState(BlendStateType type);
		std::shared_ptr<InputLayout> CreateInputLayout(InputLayoutType input_layout_type, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength);
		const DepthStencilState& GetOrCreateDepthStencilState(DepthStencilStateType type);

		// PrimitiveTopology
		void SetPrimitiveTopology(PrimitiveTopology primitive_topology);

		// InputLayout
		void SetInputLayout(InputLayoutType input_layout_type, const void* vs_blob, size_t bytecode_length);

		void SetVertexBuffers(const std::vector<BufferID>& vertex_buffer_ids, const std::vector<UInt>& vertex_strides, const std::vector<UInt>& vertex_offsets);

		void SetIndexBuffer(BufferID index_buffer_id);

		void DrawIndexed(UInt index_count);

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
		RasterizerState m_rasterizer_states[RasterizerStateType_Count];
		DepthStencilState m_depth_stencil_states[DepthStencilStateType_Count];
		BlendState m_blend_states[BlendStateType_Count];

		D3D11_RASTERIZER_DESC m_rasterizer_state_create_desc[RasterizerStateType_Count];
		std::pair<D3D11_DEPTH_STENCIL_DESC, UInt> m_depth_stencil_state_create_desc[DepthStencilStateType_Count];
		D3D11_BLEND_DESC m_blend_state_create_desc[BlendStateType_Count];

		D3D11_PRIMITIVE_TOPOLOGY m_d3d11_primitive_topology[PrimitiveTopology_Count];
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_input_element_descs[InputLayoutType_Count];
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

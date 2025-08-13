#ifndef DOLAS_RHI_H
#define DOLAS_RHI_H

#include <d3d11.h>
#include <Windows.h>
#include <vector>

#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For ID3D11Debug and ID3D11InfoQueue
#endif

#include "common/dolas_hash.h"

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
		RenderTargetView(TextureID texture_id);
		~RenderTargetView();
		ID3D11RenderTargetView* GetD3DRenderTargetView() const { return m_d3d_render_target_view; }

		// Copy / Move semantics
		RenderTargetView(const RenderTargetView& other);
		RenderTargetView& operator=(const RenderTargetView& other);
		RenderTargetView(RenderTargetView&& other) noexcept;
		RenderTargetView& operator=(RenderTargetView&& other) noexcept;

	protected:
		ID3D11RenderTargetView* m_d3d_render_target_view;
		void Release();
	};

	class DepthStencilView
	{
	public:
		DepthStencilView();
		DepthStencilView(TextureID texture_id);
		~DepthStencilView();
		ID3D11DepthStencilView* GetD3DDepthStencilView() const { return m_d3d_depth_stencil_view; }

		// Copy / Move semantics
		DepthStencilView(const DepthStencilView& other);
		DepthStencilView& operator=(const DepthStencilView& other);
		DepthStencilView(DepthStencilView&& other) noexcept;
		DepthStencilView& operator=(DepthStencilView&& other) noexcept;

	protected:
		ID3D11DepthStencilView* m_d3d_depth_stencil_view;
		void Release();

		DXGI_FORMAT ConvertToDSVFormat(DXGI_FORMAT origin_format);
	};

	struct ViewPort
	{
		D3D11_VIEWPORT m_d3d_viewport;
	};

	struct RasterizerState
	{
		ID3D11RasterizerState* m_d3d_rasterizer_state;
	};

	struct DepthStencilState
	{
		ID3D11DepthStencilState* m_d3d_depth_stencil_state;
	};

	struct BlendState
	{
		ID3D11BlendState* m_d3d_blend_state;
	};

	// 渲染硬件接口(RHI)相关定义将在这里
	class DolasRHI
	{
	public:
		DolasRHI();
		~DolasRHI();
		bool Initialize();
		void Clear();
		void Present();
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	

		HWND m_window_handle;
		int m_client_width;
		int m_client_height;
		
		void SetRenderTargetView(UInt num_views, const std::vector<RenderTargetView>& d3d11_render_target_view, const DepthStencilView& d3d11_depth_stencil_view);
		void SetViewPort(const ViewPort& viewport);
		void SetRasterizerState(const RasterizerState& rasterizer_state);
		void SetDepthStencilState(const DepthStencilState& depth_stencil_state);
		void SetBlendState(const BlendState& blend_state);
		void SetVertexShader();
		void SetPixelShader();

		ID3D11Device* m_d3d_device;
		ID3D11DeviceContext* m_d3d_immediate_context;
		IDXGISwapChain* m_swap_chain;
		ID3D11Texture2D* m_back_buffer;
		ID3D11RenderTargetView* m_back_buffer_render_target_view;

	private:
		bool InitializeWindow();
		bool InitializeD3D();
	};
}

#endif // DOLAS_RHI_H

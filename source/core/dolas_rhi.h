#ifndef DOLAS_RHI_H
#define DOLAS_RHI_H

#include <d3d11.h>
#include <Windows.h>

#if defined(DEBUG) || defined(_DEBUG)
#include <d3d11sdklayers.h>  // For ID3D11Debug and ID3D11InfoQueue
#endif

namespace Dolas
{
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
		
		ID3D11Device* m_d3d_device;
		ID3D11DeviceContext* m_d3d_immediate_context;
		IDXGISwapChain* m_swap_chain;
		ID3D11Texture2D* m_back_buffer;
		ID3D11RenderTargetView* m_render_target_view;

	private:
		bool InitializeWindow();
		bool InitializeD3D();
	};
}

#endif // DOLAS_RHI_H

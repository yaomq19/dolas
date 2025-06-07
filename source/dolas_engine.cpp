#include <Windows.h>
#include <cmath>

#include "base/dolas_base.h"
#include "dolas_engine.h"
#include "dolas_rhi.h"
#include "dolas_render_pipeline.h"

namespace Dolas
{
    DolasEngine g_dolas_engine;
    
	DolasEngine::DolasEngine()
	{
		m_rhi = DOLAS_NEW(DolasRHI);
		m_render_pipeline = DOLAS_NEW(RenderPipeline);
	}

	DolasEngine::~DolasEngine()
	{
		DOLAS_DELETE(m_rhi);
		DOLAS_DELETE(m_render_pipeline);
	}

	bool DolasEngine::Initialize()
	{
		if (!m_rhi->Initialize())
		{
			return false;
		}

		if (!m_render_pipeline->Initialize())
		{
			return false;
		}

		return true;
	}

	void DolasEngine::Run()
	{
		MSG msg = { 0 };
		
		while (msg.message != WM_QUIT)
		{
			// 处理Windows消息
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				// 渲染帧
				Render();
			}
		}
	}

	void DolasEngine::Render()
	{
		m_render_pipeline->Render(m_rhi);
	}
}// namespace Dolas


#include <Windows.h>
#include <cmath>

#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_pipeline.h"

namespace Dolas
{
    DolasEngine g_dolas_engine;
    
	DolasEngine::DolasEngine()
	{
		m_rhi = DOLAS_NEW(DolasRHI);
		m_render_pipeline_manager = DOLAS_NEW(RenderPipelineManager);
		m_mesh_manager = DOLAS_NEW(MeshManager);
		m_material_manager = DOLAS_NEW(MaterialManager);
		m_render_entity_manager = DOLAS_NEW(RenderEntityManager);
	}

	DolasEngine::~DolasEngine()
	{
		DOLAS_DELETE(m_rhi);
		DOLAS_DELETE(m_render_pipeline_manager);
		DOLAS_DELETE(m_mesh_manager);
		DOLAS_DELETE(m_material_manager);
		DOLAS_DELETE(m_render_entity_manager);
	}

	bool DolasEngine::Initialize()
	{
		DOLAS_RETURN_FALSE_IF_FALSE(m_rhi->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_pipeline_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_mesh_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_material_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_entity_manager->Initialize());
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
				Update();
				// 渲染帧
				Render();
			}
		}
	}

	void DolasEngine::Update()
	{
	}

	void DolasEngine::Render()
	{
		RenderPipeline* main_render_pipeline = m_render_pipeline_manager->GetMainRenderPipeline();
		DOLAS_RETURN_IF_NULL(main_render_pipeline);
		main_render_pipeline->Render(m_rhi);
	}	
}// namespace Dolas


#include <Windows.h>
#include <cmath>
#include <iostream>

#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "manager/dolas_render_resource_manager.h"
#include "render/dolas_render_pipeline.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_test_manager.h"

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
		m_shader_manager = DOLAS_NEW(ShaderManager);
		m_asset_manager = DOLAS_NEW(AssetManager);
		m_texture_manager = DOLAS_NEW(TextureManager);
		m_test_manager = DOLAS_NEW(TestManager);
		m_render_resource_manager = DOLAS_NEW(RenderResourceManager);
	}

	DolasEngine::~DolasEngine()
	{
		DOLAS_DELETE(m_rhi);
		DOLAS_DELETE(m_render_pipeline_manager);
		DOLAS_DELETE(m_mesh_manager);
		DOLAS_DELETE(m_material_manager);
		DOLAS_DELETE(m_render_entity_manager);
		DOLAS_DELETE(m_shader_manager);
		DOLAS_DELETE(m_asset_manager);
		DOLAS_DELETE(m_texture_manager);
		DOLAS_DELETE(m_test_manager);
		DOLAS_DELETE(m_render_resource_manager);
	}

	bool DolasEngine::Initialize()
	{
		DOLAS_RETURN_FALSE_IF_FALSE(m_rhi->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_pipeline_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_mesh_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_material_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_entity_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_shader_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_asset_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_texture_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_test_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_resource_manager->Initialize());
		return true;
	}

	void DolasEngine::Clear()
	{
		m_rhi->Clear();
		m_render_pipeline_manager->Clear();
		m_mesh_manager->Clear();
		m_material_manager->Clear();
		m_render_entity_manager->Clear();
		m_shader_manager->Clear();
		m_asset_manager->Clear();
		m_texture_manager->Clear();
		m_test_manager->Clear();
		m_render_resource_manager->Clear();
	}

	void DolasEngine::Run()
	{
		MSG msg = { 0 };
		
		while (msg.message != WM_QUIT)
		{
			// handle windows message
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Update();
				// render frame
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

	void DolasEngine::Test()
	{
		std::string str = "LUT_TExuter";
		StringID string_id = STRING_ID(str);
		std::cout << "String: " << str << ", ID: " << string_id << std::endl;

		std::string new_str = ID_TO_STRING(string_id);
		if (new_str == str)
		{
			std::cout << "String ID to String conversion successful: " << new_str << std::endl;
		}
		else
		{
			std::cout << "String ID to String conversion failed!" << new_str << std::endl;
		}
	}
}// namespace Dolas


#include <Windows.h>
#include <cmath>
#include <iostream>

#include <tracy/Tracy.hpp>

#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "manager/dolas_render_object_manager.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "manager/dolas_render_resource_manager.h"
#include "render/dolas_render_pipeline.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_test_manager.h"
#include "manager/dolas_buffer_manager.h"
#include "manager/dolas_render_primitive_manager.h"
#include "manager/dolas_render_state_manager.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
#include "manager/dolas_render_camera_manager.h"
#include "manager/dolas_render_scene_manager.h"
#include "manager/dolas_input_manager.h"
#include "manager/dolas_task_manager.h"

using Dolas::TaskGUID;
namespace Dolas
{
    DolasEngine g_dolas_engine;
    
	DolasEngine::DolasEngine()
	{
		m_log_system_manager = DOLAS_NEW(LogSystemManager);
		m_rhi = DOLAS_NEW(DolasRHI);
		m_render_pipeline_manager = DOLAS_NEW(RenderPipelineManager);
		m_mesh_manager = DOLAS_NEW(MeshManager);
		m_material_manager = DOLAS_NEW(MaterialManager);
		m_render_entity_manager = DOLAS_NEW(RenderEntityManager);
		m_render_object_manager = DOLAS_NEW(RenderObjectManager);
		m_shader_manager = DOLAS_NEW(ShaderManager);
		m_asset_manager = DOLAS_NEW(AssetManager);
		m_texture_manager = DOLAS_NEW(TextureManager);
		m_test_manager = DOLAS_NEW(TestManager);
		m_render_resource_manager = DOLAS_NEW(RenderResourceManager);
		m_render_primitive_manager = DOLAS_NEW(RenderPrimitiveManager);
		m_buffer_manager = DOLAS_NEW(BufferManager);
		m_render_state_manager = DOLAS_NEW(RenderStateManager);
		m_render_view_manager = DOLAS_NEW(RenderViewManager);
		m_render_camera_manager = DOLAS_NEW(RenderCameraManager);
		m_render_scene_manager = DOLAS_NEW(RenderSceneManager);
		m_input_manager = DOLAS_NEW(InputManager);
		m_task_manager = DOLAS_NEW(TaskManager);
	}

	DolasEngine::~DolasEngine()
	{
		DOLAS_DELETE(m_rhi);
		DOLAS_DELETE(m_render_pipeline_manager);
		DOLAS_DELETE(m_mesh_manager);
		DOLAS_DELETE(m_material_manager);
		DOLAS_DELETE(m_render_entity_manager);
		DOLAS_DELETE(m_render_object_manager);
		DOLAS_DELETE(m_shader_manager);
		DOLAS_DELETE(m_asset_manager);
		DOLAS_DELETE(m_texture_manager);
		DOLAS_DELETE(m_test_manager);
		DOLAS_DELETE(m_render_resource_manager);
		DOLAS_DELETE(m_render_primitive_manager);
		DOLAS_DELETE(m_buffer_manager);
		DOLAS_DELETE(m_render_state_manager);
		DOLAS_DELETE(m_render_view_manager);
		DOLAS_DELETE(m_render_camera_manager);
		DOLAS_DELETE(m_render_scene_manager);
		DOLAS_DELETE(m_input_manager);
		DOLAS_DELETE(m_task_manager);
		DOLAS_DELETE(m_log_system_manager);
	}

	bool DolasEngine::Initialize()
	{
		// 首先初始化日志系统
		DOLAS_RETURN_FALSE_IF_FALSE(m_log_system_manager->Initialize());
		
		DOLAS_RETURN_FALSE_IF_FALSE(m_rhi->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_pipeline_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_mesh_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_material_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_entity_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_object_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_shader_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_asset_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_texture_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_test_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_resource_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_primitive_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_buffer_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_state_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_camera_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_scene_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_view_manager->Initialize());
		// 初始化输入管理器（需要在RHI初始化之后，因为需要窗口句柄）
		DOLAS_RETURN_FALSE_IF_FALSE(m_input_manager->Initialize(m_rhi->m_window_handle));
		// 初始化任务管理器
		DOLAS_RETURN_FALSE_IF_FALSE(m_task_manager->Initialize());
		return true;
	}

	void DolasEngine::Clear()
	{
		m_rhi->Clear();
		m_render_pipeline_manager->Clear();
		m_mesh_manager->Clear();
		m_material_manager->Clear();
		m_render_entity_manager->Clear();
		m_render_object_manager->Clear();
		m_shader_manager->Clear();
		m_asset_manager->Clear();
		m_texture_manager->Clear();
		m_test_manager->Clear();
		m_render_resource_manager->Clear();
		m_render_primitive_manager->Clear();
		m_buffer_manager->Clear();
		m_render_state_manager->Clear();
		m_render_view_manager->Clear();
		m_render_camera_manager->Clear();
		m_render_scene_manager->Clear();
		m_input_manager->Clear();
		m_task_manager->Clear();
		
		// 最后清理日志系统
		m_log_system_manager->Clear();
	}

	void TickLogic()
	{

	}

	void DolasEngine::Run()
	{
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			FrameMark;
			// 处理所有当前的窗口消息（非阻塞）
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			TaskGUID logic_task_guid = m_task_manager->EnqueueTask(&DolasEngine::TickLogic, this, 1.0f);
			// render frame
			TickRender(1.0f);
			m_task_manager->WaitForTask(logic_task_guid);
			m_frame_count++;
		}
	}

	void DolasEngine::TickRender(Float delta_time)
	{
		RenderView* main_render_view = g_dolas_engine.m_render_view_manager->GetMainRenderView();
		DOLAS_RETURN_IF_NULL(main_render_view);
		main_render_view->Render(m_rhi);
	}

	void DolasEngine::TickLogic(Float delta_time)
	{
		ZoneScoped;
		// 更新输入系统
		m_input_manager->Tick();

		// 更新相机管理器（包含输入处理）
		m_render_camera_manager->Update(delta_time);
	}

	void DolasEngine::Test()
	{
		std::cout << "String ID to String conversion failed!" << std::endl;
	}
}// namespace Dolas


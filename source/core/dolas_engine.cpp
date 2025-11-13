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
#include "manager/dolas_geometry_manager.h"
#include "manager/dolas_tick_manager.h"
#include "manager/dolas_imgui_manager.h"
#include "manager/dolas_debug_draw_manager.h"
#include "manager/dolas_timer_manager.h"
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
		m_geometry_manager = DOLAS_NEW(GeometryManager);
		m_tick_manager = DOLAS_NEW(TickManager);
		m_imgui_manager = DOLAS_NEW(ImGuiManager);
		m_debug_draw_manager = DOLAS_NEW(DebugDrawManager);
		m_timer_manager = DOLAS_NEW(TimerManager);
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
		DOLAS_DELETE(m_geometry_manager);
		DOLAS_DELETE(m_log_system_manager);
		DOLAS_DELETE(m_tick_manager);
		DOLAS_DELETE(m_imgui_manager);
		DOLAS_DELETE(m_debug_draw_manager);
		DOLAS_DELETE(m_timer_manager);
	}

	bool DolasEngine::Initialize()
	{
		// First, initialize the logging system
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
		// Initialize the input manager (must be done after RHI initialization, as it requires a window handle)
		DOLAS_RETURN_FALSE_IF_FALSE(m_input_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_task_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_geometry_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_tick_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_imgui_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_debug_draw_manager->Initialize());
		DOLAS_RETURN_FALSE_IF_FALSE(m_timer_manager->Initialize());
	
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
		m_geometry_manager->Clear();
		// Finally, clean up the log system
		m_log_system_manager->Clear();
		m_tick_manager->Clear();
		m_imgui_manager->Clear();
		m_debug_draw_manager->Clear();
		m_timer_manager->Clear();
	}

	void DolasEngine::Run()
	{
		MSG msg = { 0 };
		while (msg.message != WM_QUIT)
		{
			FrameMark;

			// Limit the maximum time for message processing (microseconds)
			const auto start_time = std::chrono::high_resolution_clock::now();
			const auto max_message_time = std::chrono::microseconds(500); // 0.5ms

			// Process all current window messages (non blocking)
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					break;
				}
					
				TranslateMessage(&msg);
				DispatchMessage(&msg);

		// Check if timeout exceeded
		auto current_time = std::chrono::high_resolution_clock::now();
		if (current_time - start_time > max_message_time)
		{
			break; // Leave remaining messages for next frame
		}
	}
	
	// Update timing information (includes frame rate limiting)
	m_timer_manager->Tick();
	m_tick_manager->Tick(m_timer_manager->GetDeltaTime());
	}
}
}// namespace Dolas


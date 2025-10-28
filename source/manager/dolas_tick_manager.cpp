#include "manager/dolas_tick_manager.h"
#include "core/dolas_engine.h"
#include "manager/dolas_task_manager.h"
#include "manager/dolas_input_manager.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
#include "manager/dolas_render_camera_manager.h"
#include "manager/dolas_imgui_manager.h"
namespace Dolas
{
	using Dolas::TaskGUID;

    TickManager::TickManager()
    {
    }

    TickManager::~TickManager()
    {
    }

    bool TickManager::Initialize()
    {
        return true;
    }

    bool TickManager::Clear()
    {
        return true;
    }

    void TickManager::Tick(Float delta_time)
    {
        TaskGUID logic_task_guid = g_dolas_engine.m_task_manager->EnqueueTask(&TickManager::TickLogic, this, delta_time);
        // render frame
        TickRender(delta_time);
        g_dolas_engine.m_task_manager->WaitForTask(logic_task_guid);
        m_frame_count++;
    }

    void TickManager::TickRender(Float delta_time)
    {
        g_dolas_engine.m_imgui_manager->Tick();

        RenderView* main_render_view = g_dolas_engine.m_render_view_manager->GetMainRenderView();
		DOLAS_RETURN_IF_NULL(main_render_view);
		main_render_view->Render(g_dolas_engine.m_rhi);
    }

    void TickManager::TickLogic(Float delta_time)
    {
        ZoneScoped;
		// 更新输入系统
		g_dolas_engine.m_input_manager->Tick();

		// 更新相机管理器（包含输入处理）
		g_dolas_engine.m_render_camera_manager->Update(delta_time);
    }
}
#include "manager/dolas_tick_manager.h"
#include "dolas_engine.h"
#include "manager/dolas_task_manager.h"
#include "manager/dolas_input_manager.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
#include "manager/dolas_render_camera_manager.h"
#include "manager/dolas_imgui_manager.h"
#include "manager/dolas_debug_draw_manager.h"
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
        TaskGUID logic_task_guid = g_dolas_engine.m_task_manager->EnqueueTask(&TickManager::TickLogicThread, this, delta_time);
        // render frame
        TickRenderThread(delta_time);
        g_dolas_engine.m_task_manager->WaitForTask(logic_task_guid);
    }

    void TickManager::TickRenderThread(Float delta_time)
    {
        TickPreRender(delta_time);
        TickRender(delta_time);
        TickPostRender(delta_time);
    }

    void TickManager::TickLogicThread(Float delta_time)
    {
        TickPreLogic(delta_time);
        TickLogic(delta_time);
        TickPostLogic(delta_time);
    }

    void TickManager::TickPreRender(Float delta_time)
    {
        g_dolas_engine.m_imgui_manager->TickPreRender();
    }

    void TickManager::TickRender(Float delta_time)
    {
        RenderView* main_render_view = g_dolas_engine.m_render_view_manager->GetMainRenderView();
		DOLAS_RETURN_IF_NULL(main_render_view);
		main_render_view->Render(g_dolas_engine.m_rhi);
    }

    void TickManager::TickPostRender(Float delta_time)
    {
        g_dolas_engine.m_debug_draw_manager->TickPostRender(delta_time);
    }

    void TickManager::TickPreLogic(Float delta_time)
    {
    }

    void TickManager::TickLogic(Float delta_time)
    {
        // update input manager
		g_dolas_engine.m_input_manager->Tick();
		// update render camera manager
		g_dolas_engine.m_render_camera_manager->Update(delta_time);
    }

    void TickManager::TickPostLogic(Float delta_time)
    {
    }
}
#include "base/dolas_base.h"
#include "manager/dolas_render_camera_manager.h"
#include "render/dolas_render_camera.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "core/dolas_math.h"
#include "manager/dolas_input_manager.h"
#include <iostream>
#include <algorithm>
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
namespace Dolas
{
    RenderCameraManager::RenderCameraManager()
    {
    }

    RenderCameraManager::~RenderCameraManager()
    {
    }

    bool RenderCameraManager::Initialize()
    {
        return true;
    }

    bool RenderCameraManager::Clear()
    {
        for (auto it = m_render_cameras.begin(); it != m_render_cameras.end(); ++it)
        {
            RenderCamera* camera = it->second;
            if (camera)
            {
                DOLAS_DELETE(camera);
            }
        }
        m_render_cameras.clear();
        return true;
    }

    void RenderCameraManager::Update(Float delta_time)
    {
        ProcessInput(delta_time);
        
        for (auto iter : m_render_cameras)
        {
			// iter.second->TestRotate(delta_time);
        }
    }

    void RenderCameraManager::ProcessInput(Float delta_time)
    {
        // 获取主相机（假设使用第一个相机作为主相机）
        DOLAS_RETURN_IF_FALSE(!m_render_cameras.empty());
        RenderCamera* main_camera = m_render_cameras.begin()->second;
		DOLAS_RETURN_IF_NULL(main_camera);

        // 只有在鼠标被捕获时才处理相机控制
		DOLAS_RETURN_IF_FALSE(g_dolas_engine.m_input_manager->IsMouseCaptured());

		// 处理鼠标滚轮输入（相机速度）
		float wheel_delta = g_dolas_engine.m_input_manager->GetMouseWheelDelta();
		main_camera->ProcessWheelDelta(wheel_delta);
        g_dolas_engine.m_input_manager->ResetMouseWheelDelta();

        // 处理鼠标输入（相机旋转）
        Vector2 mouse_delta = g_dolas_engine.m_input_manager->GetMouseDelta();
        main_camera->ProcessMouseInput(mouse_delta.x, mouse_delta.y);
            
        // 处理键盘输入（相机移动）
        bool move_forward = g_dolas_engine.m_input_manager->IsKeyDown('W');
        bool move_backward = g_dolas_engine.m_input_manager->IsKeyDown('S');
        bool move_left = g_dolas_engine.m_input_manager->IsKeyDown('A');
        bool move_right = g_dolas_engine.m_input_manager->IsKeyDown('D');
        bool move_up = g_dolas_engine.m_input_manager->IsKeyDown(VK_SPACE);
        bool move_down = g_dolas_engine.m_input_manager->IsKeyDown(VK_SHIFT);

        main_camera->ProcessKeyboardInput(move_forward, move_backward, move_left, move_right,
                                        move_up, move_down, delta_time);
    }

    RenderCamera* RenderCameraManager::GetRenderCameraByID(RenderCameraID id)
    {
        if (m_render_cameras.find(id) == m_render_cameras.end()) return nullptr;
        return m_render_cameras[id];
    }

    Bool RenderCameraManager::CreateRenderCameraByID(RenderCameraID render_camera_id, const std::string& file_name)
    {
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_cameras.find(render_camera_id) == m_render_cameras.end());

        CameraRSD* camera_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<CameraRSD>(PathUtils::GetCameraDir() + file_name);
        DOLAS_RETURN_FALSE_IF_NULL(camera_rsd);

		RenderCamera* render_camera = nullptr;

		if (camera_rsd->camera_perspective_type == "perspective")
		{
            RenderCameraPerspective* perspective_render_camera = DOLAS_NEW(RenderCameraPerspective);
            perspective_render_camera->BuildFromRsd(camera_rsd);
            render_camera = perspective_render_camera;
		}
		else if (camera_rsd->camera_perspective_type == "orthographic")
		{
			RenderCameraOrthographic* ortho_render_camera = DOLAS_NEW(RenderCameraOrthographic);
			ortho_render_camera->BuildFromRsd(camera_rsd);
            render_camera = ortho_render_camera;
		}
		else
		{
			LOG_ERROR("Unknown camera perspective type: {0}", camera_rsd->camera_perspective_type);
			return false;
		}
		DOLAS_RETURN_FALSE_IF_NULL(render_camera);
		m_render_cameras[render_camera_id] = render_camera;

		return true;
    }

    void RenderCameraManager::DumpCameraInfo()
    {
        // 只打印 main camera 的信息
        RenderView* main_render_view = g_dolas_engine.m_render_view_manager->GetMainRenderView();
		DOLAS_RETURN_IF_NULL(main_render_view);
        RenderCameraID main_render_camera_id = main_render_view->GetRenderCameraID();
        RenderCamera* main_render_camera = g_dolas_engine.m_render_camera_manager->GetRenderCameraByID(main_render_camera_id);
		DOLAS_RETURN_IF_NULL(main_render_camera);
        main_render_camera->PrintDebugInfo();
    }
} // namespace Dolas



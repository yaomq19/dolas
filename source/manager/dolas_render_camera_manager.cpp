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
#include "nlohmann/json.hpp"
using json = nlohmann::json;
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
    static const Float default_move_speed = 0.03f;
    static const Float min_move_speed = 0.001f;
    static const Float max_move_speed = 0.1f;
    static const Float wheel_affect_scale = 0.01f;

    void RenderCameraManager::ProcessInput(Float delta_time)
    {
        // 获取主相机（假设使用第一个相机作为主相机）
        DOLAS_RETURN_IF_FALSE(!m_render_cameras.empty());
        
        RenderCamera* main_camera = m_render_cameras.begin()->second;
		DOLAS_RETURN_IF_NULL(main_camera);

        // 移动速度（静态变量，保持在函数调用之间）
        static Float move_speed = default_move_speed;

        // 处理鼠标滚轮（不需要鼠标捕获就可以使用）
        float wheel_delta = g_dolas_engine.m_input_manager->GetMouseWheelDelta();
        if (wheel_delta != 0.0f)
        {
            // 示例：输出滚轮值
            std::cerr << "Mouse Wheel Delta: " << wheel_delta << std::endl;
            
            move_speed += wheel_delta * wheel_affect_scale;
            // 限制移动速度范围
            if (move_speed < min_move_speed) move_speed = min_move_speed;
            if (move_speed > max_move_speed) move_speed = max_move_speed;
            std::cerr << "New move speed: " << move_speed << std::endl;
        }
        g_dolas_engine.m_input_manager->SetMouseWheelDelta(0.0f);

        // 只有在鼠标被捕获时才处理相机控制
		DOLAS_RETURN_IF_FALSE(g_dolas_engine.m_input_manager->IsMouseCaptured());

        // 处理鼠标输入（相机旋转）
        Vector2 mouse_delta = g_dolas_engine.m_input_manager->GetMouseDelta();
		static Float sensitivity = 0.1f; // 可以根据需要调整灵敏度
        if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f)
        {
            main_camera->ProcessMouseInput(mouse_delta.x, mouse_delta.y, sensitivity);
        }
            
        // 处理键盘输入（相机移动）
        bool move_forward = g_dolas_engine.m_input_manager->IsKeyDown('W');
        bool move_backward = g_dolas_engine.m_input_manager->IsKeyDown('S');
        bool move_left = g_dolas_engine.m_input_manager->IsKeyDown('A');
        bool move_right = g_dolas_engine.m_input_manager->IsKeyDown('D');
        bool move_up = g_dolas_engine.m_input_manager->IsKeyDown(VK_SPACE);
        bool move_down = g_dolas_engine.m_input_manager->IsKeyDown(VK_SHIFT);

        main_camera->ProcessKeyboardInput(move_forward, move_backward, move_left, move_right,
                                        move_up, move_down, delta_time, move_speed);
    }

    RenderCamera* RenderCameraManager::GetRenderCameraByID(RenderCameraID id)
    {
        if (m_render_cameras.find(id) == m_render_cameras.end()) return nullptr;
        return m_render_cameras[id];
    }

    Bool RenderCameraManager::CreateRenderCameraByID(RenderCameraID render_camera_id, const std::string& file_name)
    {
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_cameras.find(render_camera_id) == m_render_cameras.end());

        CameraAsset* camera_asset = g_dolas_engine.m_asset_manager->GetCameraAsset(file_name);
        DOLAS_RETURN_FALSE_IF_NULL(camera_asset);
		RenderCamera* render_camera = nullptr;

		if (camera_asset->perspective_type == "perspective")
		{
            RenderCameraPerspective* perspective_render_camera = DOLAS_NEW(RenderCameraPerspective);
            perspective_render_camera->BuildFromAsset(camera_asset);
            render_camera = perspective_render_camera;
		}
		else if (camera_asset->perspective_type == "orthographic")
		{
			RenderCameraOrthographic* ortho_render_camera = DOLAS_NEW(RenderCameraOrthographic);
			ortho_render_camera->BuildFromAsset(camera_asset);
            render_camera = ortho_render_camera;
		}
		else
		{
			std::cerr << "Unknown camera perspective type: " << camera_asset->perspective_type << std::endl;
			return false;
		}
		DOLAS_RETURN_FALSE_IF_NULL(render_camera);
		m_render_cameras[render_camera_id] = render_camera;

		return true;
    }
} // namespace Dolas



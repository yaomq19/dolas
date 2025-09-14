#include "base/dolas_base.h"
#include "manager/dolas_render_camera_manager.h"
#include "render/dolas_render_camera.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "core/dolas_math.h"
#include "manager/dolas_input_manager.h"

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

    void RenderCameraManager::Tick(Float delta_time)
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
        if (m_render_cameras.empty()) return;
        
        RenderCamera* main_camera = m_render_cameras.begin()->second;
        if (!main_camera) return;
        
        // 处理鼠标右键点击来切换鼠标捕获状态
        if (g_input_manager.IsMouseButtonDown(VK_RBUTTON))
        {
            g_input_manager.CaptureMouse(!g_input_manager.IsMouseCaptured());
        }
        
        // 只有在鼠标被捕获时才处理相机控制
        if (g_input_manager.IsMouseCaptured())
        {
            // 处理鼠标输入（相机旋转）
            Vector2 mouse_delta = g_input_manager.GetMouseDelta();
			Float sensitivity = 0.1f; // 可以根据需要调整灵敏度
            if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f)
            {
                main_camera->ProcessMouseInput(mouse_delta.x, mouse_delta.y, sensitivity);
            }
            
            // 处理键盘输入（相机移动）
            bool move_forward = g_input_manager.IsKeyDown('W');
            bool move_backward = g_input_manager.IsKeyDown('S');
            bool move_left = g_input_manager.IsKeyDown('A');
            bool move_right = g_input_manager.IsKeyDown('D');
            bool move_up = g_input_manager.IsKeyDown(VK_SPACE);
            bool move_down = g_input_manager.IsKeyDown(VK_SHIFT);
            
			Float move_speed = 0.05f; // 可以根据需要调整移动速度

            main_camera->ProcessKeyboardInput(move_forward, move_backward, move_left, move_right,
                                            move_up, move_down, delta_time, move_speed);
        }
        
        // ESC键释放鼠标捕获
        if (g_input_manager.IsKeyPressed(VK_ESCAPE))
        {
            g_input_manager.CaptureMouse(false);
        }
    }

    RenderCamera* RenderCameraManager::GetRenderCameraByID(RenderCameraID id)
    {
        if (m_render_cameras.find(id) == m_render_cameras.end()) return nullptr;
        return m_render_cameras[id];
    }

    Bool RenderCameraManager::CreateRenderCameraByID(RenderCameraID render_camera_id)
    {
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_cameras.find(render_camera_id) == m_render_cameras.end());
		/*RenderCamera* render_camera = new RenderCameraOrthographic(
			Vector3(0.0, -5.0, 0),
			Vector3(0.0, 1.0, 0.0),
			Vector3(0.0, 0.0, 1.0),
            0.0f,
            8.0f,
			2.0f,
            2.0f
		);*/

		RenderCamera* render_camera = new RenderCameraPerspective(
			Vector3(0.0, -5.0, 0),  // position
			Vector3(0.0, 1.0, 0.0),  // forward
			Vector3(0.0, 0.0, 1.0),  // up
			0.1f,   // near_plane
			8.0f,   // far_plane
			90.0f,   // fov(in degree)
            (Float)g_dolas_engine.m_rhi->m_client_width / (Float)g_dolas_engine.m_rhi->m_client_height    // aspect_ratio
		);
		
        m_render_cameras[render_camera_id] = render_camera;
		return true;
    }
} // namespace Dolas



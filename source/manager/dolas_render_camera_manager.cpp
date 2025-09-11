#include "base/dolas_base.h"
#include "manager/dolas_render_camera_manager.h"
#include "render/dolas_render_camera.h"

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
        for (auto iter : m_render_cameras)
        {
			// iter.second->TestRotate(delta_time);
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
		RenderCamera* render_camera = new RenderCameraPerspective(
			Vector3(0.0, -5.0, 0),
			Vector3(0.0, 1.0, 0.0),
			Vector3(0.0, 0.0, 1.0),
			0.1f, 1000.0f, 45.0f, 16.0f / 9.0f
		);
        m_render_cameras[render_camera_id] = render_camera;
		return true;
    }
} // namespace Dolas



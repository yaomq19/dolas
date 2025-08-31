#include "base/dolas_base.h"
#include "manager/dolas_render_camera_manager.h"
#include "render/dolas_render_camera.h"

namespace Dolas
{
    const RenderCameraID RenderCameraManager::RENDER_CAMERA_ID_MAIN = STRING_ID(main_render_camera);

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

    RenderCamera* RenderCameraManager::GetRenderCamera(RenderCameraID id)
    {
        if (m_render_cameras.find(id) == m_render_cameras.end()) return nullptr;
        return m_render_cameras[id];
    }

    Bool RenderCameraManager::CreateRenderCameraByID(RenderCameraID render_camera_id)
    {
		DOLAS_RETURN_FALSE_IF_FALSE(m_render_cameras.find(render_camera_id) == m_render_cameras.end());
        RenderCamera* render_camera = DOLAS_NEW(RenderCamera);
        DOLAS_RETURN_FALSE_IF_FALSE(render_camera->Initialize());
        m_render_cameras[render_camera_id] = render_camera;
		return true;
    }
} // namespace Dolas



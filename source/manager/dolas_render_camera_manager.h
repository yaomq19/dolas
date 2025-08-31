#ifndef DOLAS_RENDER_CAMERA_MANAGER_H
#define DOLAS_RENDER_CAMERA_MANAGER_H

#include <unordered_map>
#include <string>

#include "common/dolas_hash.h"

namespace Dolas
{
    class RenderCamera;
    class RenderCameraManager
    {
    public:
        RenderCameraManager();
        ~RenderCameraManager();

        bool Initialize();
        bool Clear();

        RenderCamera* GetRenderCamera(RenderCameraID id);
        Bool CreateRenderCameraByID(RenderCameraID render_camera_id);
    private:
        std::unordered_map<RenderCameraID, RenderCamera*> m_render_cameras;
        const static RenderCameraID RENDER_CAMERA_ID_MAIN;
    };// class RenderCameraManager
}// namespace Dolas

#endif // DOLAS_RENDER_CAMERA_MANAGER_H



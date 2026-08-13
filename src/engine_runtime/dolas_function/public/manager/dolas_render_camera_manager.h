#ifndef DOLAS_RENDER_CAMERA_MANAGER_H
#define DOLAS_RENDER_CAMERA_MANAGER_H

#include <unordered_map>
#include <string>

#include "dolas_hash.h"

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
        void Update(Float delta_time);
        RenderCamera* GetRenderCameraByID(RenderCameraID id);
        Bool CreateRenderCameraByID(RenderCameraID render_camera_id, const std::string& file_name);
        
        // 输入处理
        void ProcessInput(Float delta_time);

        void DumpCameraInfo();
    private:
        std::unordered_map<RenderCameraID, RenderCamera*> m_render_cameras;
    };// class RenderCameraManager
}// namespace Dolas

#endif // DOLAS_RENDER_CAMERA_MANAGER_H



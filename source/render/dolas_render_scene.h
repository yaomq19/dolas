#ifndef DOLAS_RENDER_SCENE_H
#define DOLAS_RENDER_SCENE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>
#include "common/dolas_hash.h"
#include "render/dolas_transform.h"
namespace Dolas
{

    class RenderScene
    {
        friend class RenderSceneManager;
    public:
        RenderScene();
        ~RenderScene();

        bool Initialize();
        bool Clear();
        Bool BuildFromFile(const std::string& file_name);
    private:
        std::vector<RenderObjectID> m_render_objects;
    }; // class RenderScene
} // namespace Dolas

#endif // DOLAS_RENDER_SCENE_H 
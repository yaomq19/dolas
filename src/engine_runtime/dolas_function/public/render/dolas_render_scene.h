#ifndef DOLAS_RENDER_SCENE_H
#define DOLAS_RENDER_SCENE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>
#include "dolas_hash.h"
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
        // Asset description parsing and entity creation are owned by RenderSceneManager.
		const std::vector<RenderEntityID>& GetRenderEntities() const { return m_render_entities; }
    private:
        std::vector<RenderEntityID> m_render_entities;
    }; // class RenderScene
} // namespace Dolas

#endif // DOLAS_RENDER_SCENE_H

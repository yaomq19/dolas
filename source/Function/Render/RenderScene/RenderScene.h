#ifndef RENDERSCENE_H
#define RENDERSCENE_H

#include <vector>

#include "Function/Render/RenderEntity/RenderEntity.h"
namespace Dolas
{
    class RenderScene
    {
    public:
        RenderScene();
        ~RenderScene();
        bool Initialize();

        std::vector<RenderEntity*> m_render_entities;
    private:
    };
};

#endif // RENDERSCENE_H
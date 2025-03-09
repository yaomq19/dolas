#include "Function/Render/RenderScene/RenderScene.h"
#include "Function/Render/RenderEntity/RenderEntity.h"
#include "Function/Render/RenderEntity/TriangleRenderEntity/TriangleRenderEntity.h"
namespace Dolas
{
    RenderScene::RenderScene()
    {
    }

    RenderScene::~RenderScene()
    {
    }

    bool RenderScene::Initialize()
    {
        TriangleRenderEntity* triangle = new TriangleRenderEntity();
        m_render_entities.push_back(triangle);
        return true;
    }
}

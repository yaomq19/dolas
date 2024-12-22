#include <d3dcompiler.h>
#include "dolas_render_pipeline.h"

namespace Dolas
{
RenderPipeline::RenderPipeline()
{

}

RenderPipeline::~RenderPipeline()
{

}

bool RenderPipeline::initialize(RHI* rhi)
{
    m_rhi = rhi;
    return true;
}


void RenderPipeline::render(Scene* scene)
{
    std::vector<RenderEntity*> render_entitys = scene->getRenderEntitys();
    for (auto render_entity : render_entitys)
    {
        // Do rendering
        render_entity->render(m_rhi);
    }

}

}
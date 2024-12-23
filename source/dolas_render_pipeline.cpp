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

bool RenderPipeline::initialize()
{
    m_rhi.initialize();
    return true;
}


void RenderPipeline::render(const Scene& scene)
{
    std::vector<RenderEntity*> render_entitys = scene.m_render_entitys;
    for (auto render_entity : render_entitys)
    {
        // Do rendering
        render_entity->render(m_rhi);
    }

}

}
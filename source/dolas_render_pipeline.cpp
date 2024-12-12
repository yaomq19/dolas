#include <iostream>
#include "dolas_render_pipeline.h"
namespace Dolas
{
RenderPipeline::RenderPipeline()
{

}

RenderPipeline::~RenderPipeline()
{

}

void RenderPipeline::render(const Scene* const scene)
{
    // Do nothing
    std::cout << "rendering......" << std::endl;
}

}
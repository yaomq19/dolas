#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include "dolas_common.h"
#include "dolas_scene.h"
namespace Dolas
{
class RenderPipeline
{
public:
    RenderPipeline();
    ~RenderPipeline();
    void render(const Scene* const scene);
};
}
#endif
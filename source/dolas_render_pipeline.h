#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include "dolas_common.h"
#include "dolas_scene.h"
#include "d3d11.h"
#include "dolas_rhi.h"
namespace Dolas
{
class RenderPipeline
{
public:
    RenderPipeline();
    ~RenderPipeline();
    bool initialize(RHI* rhi);
    void render(Scene* scene);
private:
    RHI* m_rhi;
};
}
#endif
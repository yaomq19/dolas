#ifndef DOLAS_RENDER_PIPELINE_H
#define DOLAS_RENDER_PIPELINE_H
#include "dolas_common.h"
#include "dolas_scene.h"
#include "d3d11.h"
namespace Dolas
{
class RenderPipeline
{
public:
    RenderPipeline();
    ~RenderPipeline();
    bool initialize();
    void render(const Scene* const scene);
private:
    bool initMainWindow();
    bool initDirect3D();
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_device_context;
};
}
#endif
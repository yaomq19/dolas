#ifndef DOLAS_H
#define DOLAS_H
#include <GLFW/glfw3.h>

#include "dolas_common.h"
#include "dolas_timer.h"
#include "dolas_scene.h"
#include "dolas_render_pipeline.h"
#include "dolas_rhi.h"
namespace Dolas
{
class Dolas
{
public:
    Dolas();
    ~Dolas();

    bool initialize();
    bool destroy();
    void run();
    

private:
    bool initRHI();

    Timer* m_timer;
    Scene* m_scene;
    RenderPipeline* m_render_pipeline;
    RHI* m_rhi;    
};
}


#endif
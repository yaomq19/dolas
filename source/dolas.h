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
    Dolas(UInt window_width, UInt window_height);
    ~Dolas();

    bool initialize();
    bool destroy();
    void run();
    
    GLFWwindow* m_window;
    UInt m_window_width{ 0 };
    UInt m_window_height{ 0 };
    Timer m_timer;
    Scene m_scene;
    RenderPipeline m_render_pipeline;
    
};
extern Dolas g_dolas;
}

#endif
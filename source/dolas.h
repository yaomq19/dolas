#ifndef DOLAS_H
#define DOLAS_H
#include "dolas_common.h"
#include "dolas_timer.h"
#include "dolas_scene.h"
#include "dolas_render_pipeline.h"
namespace Dolas
{
class Dolas
{
public:
    Dolas();
    ~Dolas();

    void initialize();
    bool isInitialized() const;
    void destroy();
    void run();
    
private:
    bool m_initialized;
    Timer* m_timer;
    Scene* m_scene;
    RenderPipeline* m_render_pipeline;
};
}


#endif
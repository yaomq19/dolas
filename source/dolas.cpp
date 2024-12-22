#include "dolas.h"
#include <iostream>
namespace Dolas
{

    Dolas::Dolas()
    {
        m_timer = new Timer();
        m_scene = new Scene();
        m_render_pipeline = new RenderPipeline();
        m_rhi = new RHI();
    }

    Dolas::~Dolas()
    {
        delete m_rhi;
        delete m_render_pipeline;
        delete m_scene;
        delete m_timer;
    }

    bool Dolas::initialize()
    {
        // initialize the RHI
        if (m_rhi->initialize() == false)
        {
            return false;
        }

        // initialize the render pipeline
        if (m_render_pipeline->initialize(m_rhi) == false)
        {
            return false;
        }

        return true;
    }

    bool Dolas::destroy()
    {

        return true;
    }

    void Dolas::run()
    {
        while (true)
        {
            m_timer->tick();
            Float delta_time = m_timer->getDeltaTime();
            if (delta_time > 0.0f)
            {
                m_scene->tick(delta_time);
                m_render_pipeline->render(m_scene);
            }
        }
    }

}
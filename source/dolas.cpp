#include "dolas.h"
namespace Dolas
{

    Dolas::Dolas() : m_initialized(false)
    {}

    Dolas::~Dolas()
    {
    }

    void Dolas::initialize()
    {
        m_timer = new Timer();
        m_scene = new Scene();
        m_render_pipeline = new RenderPipeline();

        m_initialized = true;
    }

    bool Dolas::isInitialized() const
    {
        return m_initialized;
    }

    void Dolas::destroy()
    {
        delete m_render_pipeline;
        delete m_scene;
        delete m_timer;
        m_initialized = false;
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
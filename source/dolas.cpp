#include "dolas.h"
#include <iostream>
namespace Dolas
{
    class Dolas g_dolas(1280, 720);
    Dolas::Dolas(UInt window_width, UInt window_height)
    {
        m_window_width = window_width;
        m_window_height = window_height;
    }

    Dolas::~Dolas()
    {
    }

    bool Dolas::initialize()
    {
		if (!glfwInit())
		{
			return false;
		}

		m_window = glfwCreateWindow(m_window_width, m_window_height, "Dolas Window", nullptr, nullptr);
		if (!m_window)
		{
			glfwTerminate();
			return false;
		}
		return true;

        // initialize the render pipeline
        if (m_render_pipeline.initialize() == false)
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
            m_timer.tick();
            Float delta_time = m_timer.getDeltaTime();
            if (delta_time > 0.0f)
            {
                m_scene.tick(delta_time);
                m_render_pipeline.render(m_scene);
            }
        }
    }

}
#include "render/dolas_render_primitive.h"

#include <iostream>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"

namespace Dolas
{
    RenderPrimitive::RenderPrimitive() :
		m_topology(PrimitiveTopology::PrimitiveTopology_TriangleList),
		m_vertex_count(0),
		m_index_count(0),
		m_index_buffer_id(BUFFER_ID_EMPTY)
    {
    }

    RenderPrimitive::~RenderPrimitive()
    {
        Clear();
    }

    bool RenderPrimitive::Clear()
    {
        m_vertex_count = 0;
        m_index_count = 0;
        m_topology = PrimitiveTopology::PrimitiveTopology_TriangleList;
        return true;
    }
}



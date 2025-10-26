#ifndef DOLAS_RENDER_PRIMITIVE_H
#define DOLAS_RENDER_PRIMITIVE_H

#include <string>
#include <memory>
#include <d3d11.h>

#include "common/dolas_hash.h"
#include "dolas_mesh.h"
namespace Dolas
{
    class DolasRHI;
    class Buffer;

    class RenderPrimitive
    {
        friend class RenderPrimitiveManager;
    public:
        RenderPrimitive();
        ~RenderPrimitive();

        bool Clear();
        void Draw(DolasRHI* rhi);

        RenderPrimitiveID m_id;
        RenderPrimitiveTopology m_topology;
		InputLayoutType m_input_layout_type;
        
		// The step size of each vertex, unit: Float
        UInt m_vertex_stride = 0;

		// vertex count
        UInt m_vertex_count = 0;
        
		// index count
        UInt m_index_count = 0;
        
		BufferID m_vertex_buffer_id;

		BufferID m_index_buffer_id;
    };// class RenderPrimitive
} // namespace Dolas

#endif // DOLAS_RENDER_PRIMITIVE_H



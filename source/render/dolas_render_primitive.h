#ifndef DOLAS_RENDER_PRIMITIVE_H
#define DOLAS_RENDER_PRIMITIVE_H

#include <string>
#include <memory>
#include <d3d11.h>

#include "common/dolas_hash.h"
#include "core/dolas_rhi_common.h"
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

        PrimitiveTopology m_topology;
		InputLayoutType m_input_layout_type;
        
		std::vector<BufferID> m_vertex_buffer_ids;
		std::vector<UInt> m_vertex_strides;
		std::vector<UInt> m_vertex_offsets;
        
		// vertex count
        UInt m_vertex_count = 0;
        
		// index count
        BufferID m_index_buffer_id;
        UInt m_index_count = 0;
		
    };// class RenderPrimitive
} // namespace Dolas

#endif // DOLAS_RENDER_PRIMITIVE_H



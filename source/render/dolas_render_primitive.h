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

        // 绘制
        void Draw(DolasRHI* rhi);

        // 属性
        const std::string& GetName() const { return m_name; }
        RenderPrimitiveID GetId() const { return m_id; }

        std::string m_name;
        RenderPrimitiveID m_id;

        BufferID m_vertex_buffer_id;
        BufferID m_index_buffer_id;

		// 每个顶点的步长，单位：Float
        UInt m_vertex_stride = 0;

		// 顶点数量
        UInt m_vertex_count = 0;
        
		// 索引数量
        UInt m_index_count = 0;
        DolasRenderPrimitiveType m_topology;
        DolasInputLayoutType m_input_layout_type;
    };// class RenderPrimitive
} // namespace Dolas

#endif // DOLAS_RENDER_PRIMITIVE_H



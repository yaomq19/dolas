#include "render/dolas_render_primitive.h"

#include <iostream>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_buffer.h"

namespace Dolas
{
    RenderPrimitive::RenderPrimitive()
        : m_id(RENDER_PRIMITIVE_ID_EMPTY),
		m_topology(RenderPrimitiveTopology::_TRIANGLE_LIST),
		m_vertex_stride(0),
		m_vertex_count(0),
		m_index_count(0),
		m_vertex_buffer_id(BUFFER_ID_EMPTY),
		m_index_buffer_id(BUFFER_ID_EMPTY)
    {
    }

    RenderPrimitive::~RenderPrimitive()
    {
        Clear();
    }

    bool RenderPrimitive::Clear()
    {
        m_vertex_buffer_id = BUFFER_ID_EMPTY;
        m_vertex_buffer_id = BUFFER_ID_EMPTY;
        m_vertex_stride = 0;
        m_vertex_count = 0;
        m_index_count = 0;
        m_topology = RenderPrimitiveTopology::_TRIANGLE_LIST;
        return true;
    }

    void RenderPrimitive::Draw(DolasRHI* rhi)
    {
        DOLAS_RETURN_IF_NULL(rhi);

		UINT stride = m_vertex_stride * sizeof(Float);
		UINT offset = 0;

        Buffer* vertex_buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(m_vertex_buffer_id);
        DOLAS_RETURN_IF_NULL(vertex_buffer);
		ID3D11Buffer* d3d_vertex_buffer = vertex_buffer->GetBuffer();

		Buffer* index_buffer = g_dolas_engine.m_buffer_manager->GetBufferByID(m_index_buffer_id);
		DOLAS_RETURN_IF_NULL(index_buffer);
		ID3D11Buffer* d3d_index_buffer = index_buffer->GetBuffer();

		rhi->m_d3d_immediate_context->IASetVertexBuffers(0, 1, &d3d_vertex_buffer, &stride, &offset);

        D3D11_PRIMITIVE_TOPOLOGY d3d_topology;
        if (m_topology == RenderPrimitiveTopology::_TRIANGLE_LIST)
        {
            d3d_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        else
        {
            return;
        }
		rhi->m_d3d_immediate_context->IASetPrimitiveTopology(d3d_topology);

		if (index_buffer)
		{
			rhi->m_d3d_immediate_context->IASetIndexBuffer(index_buffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
			rhi->m_d3d_immediate_context->DrawIndexed(m_index_count, 0, 0);
		}
		else
		{
			rhi->m_d3d_immediate_context->Draw(m_vertex_count, 0);
		}
    }
}



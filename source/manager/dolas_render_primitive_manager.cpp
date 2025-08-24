#include "manager/dolas_render_primitive_manager.h"

#include <iostream>

#include "common/dolas_hash.h"
#include "core/dolas_engine.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_render_primitive.h"

namespace Dolas
{
    RenderPrimitiveManager::RenderPrimitiveManager()
    {
    }

    RenderPrimitiveManager::~RenderPrimitiveManager()
    {
    }

    bool RenderPrimitiveManager::Initialize()
    {
        return true;
    }

    bool RenderPrimitiveManager::Clear()
    {
        m_render_primitives.clear();
        return true;
    }

    Bool RenderPrimitiveManager::BuildFromRawData(
		RenderPrimitiveID id,
		const DolasRenderPrimitiveType& render_primitive_type,
		const DolasInputLayoutType& input_layout_type,
		const std::vector<float>& vertices,
		const std::vector<unsigned int>& indices,
        RenderPrimitive* render_primitive)
    {
        render_primitive->m_id = id;
		render_primitive->m_topology = render_primitive_type;
        render_primitive->m_input_layout_type = input_layout_type;

        if (input_layout_type == DolasInputLayoutType::POS_3_UV_2_NORM_3)
        {
            render_primitive->m_vertex_stride = 8;
            render_primitive->m_vertex_count = vertices.size() / 8;
            render_primitive->m_index_count = indices.size();

			render_primitive->m_vertex_buffer_id = g_dolas_engine.m_buffer_manager->CreateVertexBuffer(render_primitive->m_vertex_count, vertices.data());
			render_primitive->m_index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer(render_primitive->m_index_count, indices.data());
        }
        else if (input_layout_type == DolasInputLayoutType::POS_3_UV_2)
        {
			render_primitive->m_vertex_stride = 5;
			render_primitive->m_vertex_count = vertices.size() / 5;
			render_primitive->m_index_count = indices.size();

			render_primitive->m_vertex_buffer_id = g_dolas_engine.m_buffer_manager->CreateVertexBuffer(render_primitive->m_vertex_count * 5 * 4, vertices.data());
			render_primitive->m_index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer(render_primitive->m_index_count * 4, indices.data());
        }
        else
        {
            return false;
        }

        return true;
    }
    RenderPrimitiveID RenderPrimitiveManager::CreateRenderPrimitive(
        RenderPrimitiveID id,
        const DolasRenderPrimitiveType& render_primitive_type,
        const DolasInputLayoutType& input_layout_type,
        const std::vector<float>& vertices,
		const std::vector<unsigned int>& indices)
    {
        RenderPrimitive* render_primitive = DOLAS_NEW(RenderPrimitive);
        Bool success = BuildFromRawData(
            id,
            render_primitive_type,
            input_layout_type,
            vertices,
            indices,
			render_primitive);

        if (success)
        {
			m_render_primitives[id] = render_primitive;
            return id;
        }

        return RENDER_PRIMITIVE_ID_EMPTY;
    }

    RenderPrimitive* RenderPrimitiveManager::GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id)
    {
        if (m_render_primitives.find(render_primitive_id) != m_render_primitives.end())
        {
            return m_render_primitives[render_primitive_id];
        }
        return nullptr;
    }
}



#include "manager/dolas_render_primitive_manager.h"

#include <iostream>

#include "common/dolas_hash.h"
#include "core/dolas_engine.h"
#include "manager/dolas_buffer_manager.h"
#include "render/dolas_render_primitive.h"
#include "manager/dolas_log_system_manager.h"
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

    RenderPrimitive* RenderPrimitiveManager::BuildFromRawData(
		RenderPrimitiveID id,
		const RenderPrimitiveTopology& render_primitive_type,
		const InputLayoutType& input_layout_type,
		const std::vector<Float>& vertices,
		const std::vector<UInt>& indices)
    {
        UInt vertex_stride = 0;

        switch (input_layout_type)
        {
        case InputLayoutType::InputLayoutType_POS_3:
            vertex_stride = 3;
            break;
        case InputLayoutType::InputLayoutType_POS_3_UV_2:
            vertex_stride = 5; // 3 + 2 = 5
            break;
        case InputLayoutType::InputLayoutType_POS_3_UV_2_NORM_3:
            vertex_stride = 8; // 3 + 2 + 3 = 8
            break;
        default:
			LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Unsupported input layout type");
			return nullptr;
        }

		UInt vertex_count = vertices.size() / vertex_stride;
		UInt index_count = indices.size();

        BufferID vertex_buffer_id = g_dolas_engine.m_buffer_manager->CreateVertexBuffer(vertices.size() * sizeof(Float), vertices.data());
		if (vertex_buffer_id == BUFFER_ID_EMPTY)
		{
			LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create vertex buffer");
			return nullptr;
		}

        BufferID index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer(indices.size() * sizeof(UInt), indices.data());
		if (index_buffer_id == BUFFER_ID_EMPTY)
		{
			LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create index buffer");
			return nullptr;

		}
        RenderPrimitive* render_primitive = DOLAS_NEW(RenderPrimitive);

		render_primitive->m_id = id;
		render_primitive->m_topology = render_primitive_type;
		render_primitive->m_input_layout_type = input_layout_type;
		render_primitive->m_vertex_stride = vertex_stride;
		render_primitive->m_vertex_count = vertex_count;
		render_primitive->m_index_count = index_count;
		render_primitive->m_vertex_buffer_id = vertex_buffer_id;
		render_primitive->m_index_buffer_id = index_buffer_id;

        return render_primitive;
    }

    Bool RenderPrimitiveManager::CreateRenderPrimitive(
        RenderPrimitiveID id,
        const RenderPrimitiveTopology& render_primitive_type,
        const InputLayoutType& input_layout_type,
        const std::vector<Float>& vertices,
		const std::vector<UInt>& indices)
    {
		RenderPrimitive* render_primitive = BuildFromRawData(id, render_primitive_type, input_layout_type, vertices, indices);

        if (render_primitive)
        {
			m_render_primitives[id] = render_primitive;
            return true;
        }
        else
        {
			LOG_ERROR("RenderPrimitiveManager::CreateRenderPrimitive: Failed to build render primitive from raw data");
            return false;
        }
    }

    RenderPrimitive* RenderPrimitiveManager::GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id) const
    {
        if (m_render_primitives.find(render_primitive_id) != m_render_primitives.end())
        {
            return m_render_primitives.at(render_primitive_id);
        }
        return nullptr;
    }

    Bool RenderPrimitiveManager::DeleteRenderPrimitiveByID(RenderPrimitiveID render_primitive_id)
    {
        auto finder = m_render_primitives.find(render_primitive_id);
		if (finder == m_render_primitives.end())
		{
			return false;
		}

        if (finder->second == nullptr)
        {
            return false;
        }

        finder->second->Clear();
        m_render_primitives.erase(finder);

        return true;
    }
}



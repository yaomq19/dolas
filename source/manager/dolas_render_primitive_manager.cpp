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
		const PrimitiveTopology& render_primitive_type,
		const InputLayoutType& input_layout_type,
		const std::vector<std::vector<Float>>& vertices,
		const std::vector<UInt>& indices)
    {
        // vertex buffers
        std::vector<BufferID> vertex_buffer_ids;
        std::vector<UInt> vertex_strides; // unit: byte
        std::vector<UInt> vertex_offsets; // unit: byte

        UInt vertex_count = 0;
        for (UInt stream_index = 0; stream_index < vertices.size(); stream_index++)
        {
            UInt vertex_stride = 0; // unit: float
            switch (input_layout_type)
            {
            case InputLayoutType::InputLayoutType_POS_3:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3");
                    return nullptr;
                }
#endif
                break;
            case InputLayoutType::InputLayoutType_POS_3_UV_2:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
                else if (stream_index == 1)
                {
                    vertex_stride = 2;
                }
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3_UV_2");
                    return nullptr;
                }
#endif
                break;
            case InputLayoutType::InputLayoutType_POS_3_UV_2_NORM_3:
                if (stream_index == 0)
                {
                    vertex_stride = 3;
                }
                else if (stream_index == 1)
                {
                    vertex_stride = 2;
                }
                else if (stream_index == 2)
                {
                    vertex_stride = 3;
				}
#ifdef DOLAS_DEBUG
                else
                {
                    LOG_ERROR("Vertex stride mismatch: POS_3_UV_2_NORM_3");
                    return nullptr;
                }
#endif
                break;
            default:
			    LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Unsupported input layout type");
			    return nullptr;
        }

		    vertex_count = vertices[stream_index].size() / vertex_stride;

            BufferID vertex_buffer_id = g_dolas_engine.m_buffer_manager->CreateVertexBuffer(vertices[stream_index]);
		    if (vertex_buffer_id == BUFFER_ID_EMPTY)
		    {
			    LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create vertex buffer");
			    return nullptr;
		    }
            vertex_buffer_ids.push_back(vertex_buffer_id);
            vertex_strides.push_back(vertex_stride * sizeof(Float)); // convert to byte
			vertex_offsets.push_back(0); // assume offset is 0
        }
        
        // index buffer
        UInt index_count = indices.size();
        BufferID index_buffer_id = g_dolas_engine.m_buffer_manager->CreateIndexBuffer(indices.size() * sizeof(UInt), indices.data());
		if (index_buffer_id == BUFFER_ID_EMPTY)
		{
			LOG_ERROR("RenderPrimitiveManager::BuildFromRawData: Failed to create index buffer");
			return nullptr;

		}
        RenderPrimitive* render_primitive = DOLAS_NEW(RenderPrimitive);

		render_primitive->m_topology = render_primitive_type;
		render_primitive->m_input_layout_type = input_layout_type;
		render_primitive->m_vertex_strides = vertex_strides;
		render_primitive->m_vertex_offsets = vertex_offsets;
		render_primitive->m_vertex_count = vertex_count;
		render_primitive->m_index_count = index_count;
		render_primitive->m_vertex_buffer_ids = vertex_buffer_ids;
		render_primitive->m_index_buffer_id = index_buffer_id;

        return render_primitive;
    }

    Bool RenderPrimitiveManager::CreateRenderPrimitive(
        RenderPrimitiveID id,
        const PrimitiveTopology& render_primitive_type,
        const InputLayoutType& input_layout_type,
        const std::vector<std::vector<Float>>& vertices,
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



#include "manager/dolas_buffer_manager.h"
#include "render/dolas_buffer.h"
#include "dolas_base.h"
#include <iostream>
#include <atomic>
#include "dolas_log_system_manager.h"
namespace Dolas
{

static std::atomic<UInt> s_next_buffer_id{ 1 };
    BufferManager::BufferManager()
    {

    }

    BufferManager::~BufferManager()
    {

    }
    
    bool BufferManager::Initialize()
    {
        return true;
    }

    bool BufferManager::Clear()
    {
        for (auto buffer_iter = m_buffers.begin(); buffer_iter != m_buffers.end(); ++buffer_iter)
        {
            Buffer* buffer = buffer_iter->second;
            if (buffer)
            {
                buffer->Release();
                DOLAS_DELETE(buffer);
            }
        }
        m_buffers.clear();
        LOG_INFO("BufferManager::Clear: All buffers cleared");
        return true;
    }

    BufferID BufferManager::CreateBuffer(BufferID buffer_id, BufferType type, BufferUsage usage, uint32_t size, uint32_t stride, const void* initial_data)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        // 创建缓冲区
        if (!buffer->CreateBuffer(type, usage, size, stride, initial_data))
        {
            buffer->Release();
            DOLAS_DELETE(buffer);
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;

        return buffer_id;
    }

    BufferID BufferManager::CreateVertexBuffer(
		const std::vector<Float>& vertex_data,
        BufferUsage usage,
        BufferID buffer_id)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        if (buffer_id == BUFFER_ID_EMPTY)
        {
            buffer_id = s_next_buffer_id.fetch_add(1, std::memory_order_relaxed);
        }
        // 创建顶点缓冲区
        if (!buffer->CreateVertexBuffer(vertex_data.size() * sizeof(Float), vertex_data.data(), usage))
        {
            buffer->Release();
            DOLAS_DELETE(buffer);
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;

        return buffer_id;
    }

    BufferID BufferManager::CreateIndexBuffer(uint32_t size, const void* initial_data, BufferUsage usage,BufferID buffer_id)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
		if (buffer_id == BUFFER_ID_EMPTY)
		{
			buffer_id = s_next_buffer_id.fetch_add(1, std::memory_order_relaxed);
		}

        // 创建索引缓冲区
        if (!buffer->CreateIndexBuffer(size, initial_data, usage))
        {
            buffer->Release();
            DOLAS_DELETE(buffer);
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;

        return buffer_id;
    }

    BufferID BufferManager::CreateConstantBuffer(BufferID buffer_id, uint32_t size, const void* initial_data, BufferUsage usage)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        // 创建常量缓冲区
        if (!buffer->CreateConstantBuffer(size, initial_data, usage))
        {
            buffer->Release();
            DOLAS_DELETE(buffer);
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;
        
        return buffer_id;
    }

    BufferID BufferManager::CreateStructuredBuffer(BufferID buffer_id, uint32_t element_count, uint32_t element_size, const void* initial_data, BufferUsage usage)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        // 创建结构化缓冲区
        if (!buffer->CreateStructuredBuffer(element_count, element_size, initial_data, usage))
        {
            LOG_ERROR("BufferManager::CreateStructuredBuffer: Failed to create structured buffer: {0}", buffer_id);
            buffer->Release();
            DOLAS_DELETE(buffer);
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;
        uint32_t total_size = element_count * element_size;
        LOG_INFO("BufferManager::CreateStructuredBuffer: Successfully created structured buffer: {0} ({1} elements, {2} bytes each, total: {3} bytes)", buffer_id, element_count, element_size, total_size);
        return buffer_id;
    }

    Buffer* BufferManager::GetBufferByID(BufferID buffer_id)
    {
        auto it = m_buffers.find(buffer_id);
        return (it != m_buffers.end()) ? it->second : nullptr;
    }

    uint32_t BufferManager::GetTotalBufferMemory() const
    {
        uint32_t total_memory = 0;
        for (const auto& pair : m_buffers)
        {
            if (pair.second)
            {
                total_memory += pair.second->GetSize();
            }
        }
        return total_memory;
    }

} // namespace Dolas

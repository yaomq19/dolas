#include "manager/dolas_buffer_manager.h"
#include "render/dolas_buffer.h"
#include "base/dolas_base.h"
#include <iostream>

namespace Dolas
{
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
        for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it)
        {
            Buffer* buffer = it->second;
            if (buffer)
            {
                buffer->Release();
            }
        }
        m_buffers.clear();
        std::cout << "BufferManager::Clear: All buffers cleared" << std::endl;
        return true;
    }

    BufferID BufferManager::CreateBuffer(BufferID buffer_id, BufferType type, BufferUsage usage, uint32_t size, uint32_t stride, const void* initial_data)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        // 创建缓冲区
        if (!buffer->CreateBuffer(type, usage, size, stride, initial_data))
        {

            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;

        return buffer_id;
    }

    BufferID BufferManager::CreateVertexBuffer(
        uint32_t size,
        const void* initial_data,
        BufferUsage usage,
        BufferID buffer_id)
    {
        // 创建缓冲区对象
        Buffer* buffer = DOLAS_NEW(Buffer);
        
        if (buffer_id == BUFFER_ID_EMPTY)
        {
            UInt random_uint = rand();
            while (m_buffers.find(random_uint) != m_buffers.end())
            {
                random_uint = rand();
            }

            buffer_id = random_uint;
        }
        // 创建顶点缓冲区
        if (!buffer->CreateVertexBuffer(size, initial_data, usage))
        {
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
			UInt random_uint = rand();
			while (m_buffers.find(random_uint) != m_buffers.end())
			{
				random_uint = rand();
			}

			buffer_id = random_uint;
		}

        // 创建索引缓冲区
        if (!buffer->CreateIndexBuffer(size, initial_data, usage))
        {
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
            std::cerr << "BufferManager::CreateStructuredBuffer: Failed to create structured buffer: " << buffer_id << std::endl;
            return BUFFER_ID_EMPTY;
        }

        m_buffers[buffer_id] = buffer;
        uint32_t total_size = element_count * element_size;
        std::cout << "BufferManager::CreateStructuredBuffer: Successfully created structured buffer: " << buffer_id 
                  << " (" << element_count << " elements, " << element_size << " bytes each, total: " << total_size << " bytes)" << std::endl;
        return buffer_id;
    }

    Buffer* BufferManager::GetBufferByID(BufferID buffer_id)
    {
		if (m_buffers.find(buffer_id) != m_buffers.end())
        {
            return m_buffers[buffer_id];
        }
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

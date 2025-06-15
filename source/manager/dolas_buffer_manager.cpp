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
            std::shared_ptr<Buffer> buffer = it->second;
            if (buffer)
            {
                buffer->Release();
            }
        }
        m_buffers.clear();
        std::cout << "BufferManager::Clear: All buffers cleared" << std::endl;
        return true;
    }

    std::shared_ptr<Buffer> BufferManager::CreateBuffer(const std::string& name, BufferType type, BufferUsage usage, uint32_t size, uint32_t stride, const void* initial_data)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建缓冲区
        if (!buffer->CreateBuffer(type, usage, size, stride, initial_data))
        {
            std::cerr << "BufferManager::CreateBuffer: Failed to create buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        std::cout << "BufferManager::CreateBuffer: Successfully created buffer: " << name << " (size: " << size << " bytes)" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::CreateVertexBuffer(const std::string& name, uint32_t size, const void* initial_data, BufferUsage usage)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateVertexBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建顶点缓冲区
        if (!buffer->CreateVertexBuffer(size, initial_data, usage))
        {
            std::cerr << "BufferManager::CreateVertexBuffer: Failed to create vertex buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        std::cout << "BufferManager::CreateVertexBuffer: Successfully created vertex buffer: " << name << " (size: " << size << " bytes)" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::CreateIndexBuffer(const std::string& name, uint32_t size, const void* initial_data, BufferUsage usage)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateIndexBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建索引缓冲区
        if (!buffer->CreateIndexBuffer(size, initial_data, usage))
        {
            std::cerr << "BufferManager::CreateIndexBuffer: Failed to create index buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        std::cout << "BufferManager::CreateIndexBuffer: Successfully created index buffer: " << name << " (size: " << size << " bytes)" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::CreateConstantBuffer(const std::string& name, uint32_t size, const void* initial_data, BufferUsage usage)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateConstantBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建常量缓冲区
        if (!buffer->CreateConstantBuffer(size, initial_data, usage))
        {
            std::cerr << "BufferManager::CreateConstantBuffer: Failed to create constant buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        
        // 常量缓冲区大小会被对齐到16字节边界
        uint32_t aligned_size = (size + 15) & ~15;
        std::cout << "BufferManager::CreateConstantBuffer: Successfully created constant buffer: " << name << " (size: " << aligned_size << " bytes, aligned from " << size << ")" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::CreateStructuredBuffer(const std::string& name, uint32_t element_count, uint32_t element_size, const void* initial_data, BufferUsage usage)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateStructuredBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建结构化缓冲区
        if (!buffer->CreateStructuredBuffer(element_count, element_size, initial_data, usage))
        {
            std::cerr << "BufferManager::CreateStructuredBuffer: Failed to create structured buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        uint32_t total_size = element_count * element_size;
        std::cout << "BufferManager::CreateStructuredBuffer: Successfully created structured buffer: " << name 
                  << " (" << element_count << " elements, " << element_size << " bytes each, total: " << total_size << " bytes)" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::CreateStagingBuffer(const std::string& name, uint32_t size, const void* initial_data)
    {
        // 检查是否已存在同名缓冲区
        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "BufferManager::CreateStagingBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return m_buffers[name];
        }

        // 创建缓冲区对象
        std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
        buffer->m_name = name;
        
        // 创建暂存缓冲区
        if (!buffer->CreateBuffer(BufferType::STAGING_BUFFER, BufferUsage::STAGING, size, 0, initial_data))
        {
            std::cerr << "BufferManager::CreateStagingBuffer: Failed to create staging buffer: " << name << std::endl;
            return nullptr;
        }

        m_buffers[name] = buffer;
        std::cout << "BufferManager::CreateStagingBuffer: Successfully created staging buffer: " << name << " (size: " << size << " bytes)" << std::endl;
        return buffer;
    }

    std::shared_ptr<Buffer> BufferManager::GetBuffer(const std::string& name)
    {
        auto it = m_buffers.find(name);
        if (it != m_buffers.end())
        {
            return it->second;
        }
        
        std::cerr << "BufferManager::GetBuffer: Buffer not found: " << name << std::endl;
        return nullptr;
    }

    bool BufferManager::HasBuffer(const std::string& name) const
    {
        return m_buffers.find(name) != m_buffers.end();
    }

    bool BufferManager::RemoveBuffer(const std::string& name)
    {
        auto it = m_buffers.find(name);
        if (it != m_buffers.end())
        {
            std::shared_ptr<Buffer> buffer = it->second;
            if (buffer)
            {
                buffer->Release();
            }
            m_buffers.erase(it);
            std::cout << "BufferManager::RemoveBuffer: Successfully removed buffer: " << name << std::endl;
            return true;
        }
        
        std::cerr << "BufferManager::RemoveBuffer: Buffer not found: " << name << std::endl;
        return false;
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

#ifndef DOLAS_BUFFER_MANAGER_H
#define DOLAS_BUFFER_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_buffer.h"

namespace Dolas
{
    class BufferManager
    {
    public:
        BufferManager();
        ~BufferManager();
        bool Initialize();
        bool Clear();

        // 通用缓冲区创建
        std::shared_ptr<Buffer> CreateBuffer(const std::string& name, BufferType type, BufferUsage usage, uint32_t size, uint32_t stride = 0, const void* initial_data = nullptr);
        
        // 特定类型缓冲区创建
        std::shared_ptr<Buffer> CreateVertexBuffer(const std::string& name, uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::IMMUTABLE);
        std::shared_ptr<Buffer> CreateIndexBuffer(const std::string& name, uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::IMMUTABLE);
        std::shared_ptr<Buffer> CreateConstantBuffer(const std::string& name, uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DYNAMIC);
        std::shared_ptr<Buffer> CreateStructuredBuffer(const std::string& name, uint32_t element_count, uint32_t element_size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DEFAULT);
        std::shared_ptr<Buffer> CreateStagingBuffer(const std::string& name, uint32_t size, const void* initial_data = nullptr);
        
        // 获取已存在的缓冲区
        std::shared_ptr<Buffer> GetBuffer(const std::string& name);
        
        // 检查缓冲区是否存在
        bool HasBuffer(const std::string& name) const;
        
        // 删除特定缓冲区
        bool RemoveBuffer(const std::string& name);

        // 获取缓冲区统计信息
        size_t GetBufferCount() const { return m_buffers.size(); }
        uint32_t GetTotalBufferMemory() const;

    private:
        std::unordered_map<std::string, std::shared_ptr<Buffer>> m_buffers;
    }; // class BufferManager
} // namespace Dolas

#endif // DOLAS_BUFFER_MANAGER_H

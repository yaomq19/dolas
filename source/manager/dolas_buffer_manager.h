#ifndef DOLAS_BUFFER_MANAGER_H
#define DOLAS_BUFFER_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_buffer.h"
#include "common/dolas_hash.h"
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
        BufferID CreateBuffer(BufferID buffer_id, BufferType type, BufferUsage usage, uint32_t size, uint32_t stride, const void* initial_data);
        
        // 特定类型缓冲区创建
        BufferID CreateVertexBuffer(
            uint32_t size,
            const void* initial_data = nullptr,
            BufferUsage usage = BufferUsage::IMMUTABLE,
            BufferID buffer_id = BUFFER_ID_EMPTY);

        BufferID CreateIndexBuffer(uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::IMMUTABLE, BufferID buffer_id = BUFFER_ID_EMPTY);
        BufferID CreateConstantBuffer(BufferID buffer_id, uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DYNAMIC);
        BufferID CreateStructuredBuffer(BufferID buffer_id, uint32_t element_count, uint32_t element_size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DEFAULT);
        
        // 获取已存在的缓冲区
        Buffer* GetBufferByID(BufferID buffer_id);
        
        // 获取缓冲区统计信息
        size_t GetBufferCount() const { return m_buffers.size(); }
        uint32_t GetTotalBufferMemory() const;

    private:
        std::unordered_map<BufferID, Buffer*> m_buffers;
    }; // class BufferManager
} // namespace Dolas

#endif // DOLAS_BUFFER_MANAGER_H

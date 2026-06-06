#ifndef DOLAS_BUFFER_H
#define DOLAS_BUFFER_H

#include <cstdint>
#include <string>

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D12Resource;

namespace Dolas
{
    enum class BufferType
    {
        VERTEX_BUFFER,
        INDEX_BUFFER,
        CONSTANT_BUFFER,
        STRUCTURED_BUFFER,
        STAGING_BUFFER,
        APPEND_CONSUME_BUFFER
    };

    enum class BufferUsage
    {
        DEFAULT,        // GPU读写，CPU不可访问
        IMMUTABLE,      // GPU只读，CPU不可访问，创建时初始化
        DYNAMIC,        // GPU只读，CPU写入
        STAGING         // CPU和GPU都可访问，用于数据传输
    };

    enum class BufferAccess
    {
        NONE = 0,
        READ = 1,
        WRITE = 2,
        READ_WRITE = 3
    };

    class Buffer
    {
        friend class BufferManager;
    public:
        Buffer();
        ~Buffer();

        bool CreateBuffer(BufferType type, BufferUsage usage, uint32_t size, uint32_t stride = 0, const void* initial_data = nullptr);
        bool CreateVertexBuffer(uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::IMMUTABLE);
        bool CreateIndexBuffer(uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::IMMUTABLE);
        bool CreateConstantBuffer(uint32_t size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DYNAMIC);
        bool CreateStructuredBuffer(uint32_t element_count, uint32_t element_size, const void* initial_data = nullptr, BufferUsage usage = BufferUsage::DEFAULT);
        
        // 数据更新
        bool UpdateData(const void* data, uint32_t size, uint32_t offset = 0);
        bool Map(void** mapped_data, BufferAccess access = BufferAccess::WRITE);
        void Unmap();
        
        // 数据读取（仅适用于Staging Buffer）
        bool ReadData(void* output_data, uint32_t size, uint32_t offset = 0);
        
        void Release();

        // Getters
        ID3D11Buffer* GetBuffer() const { return m_d3d_buffer; }
        ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shader_resource_view; }
        ID3D11UnorderedAccessView* GetUnorderedAccessView() const { return m_unordered_access_view; }
        ID3D12Resource* GetD3D12Resource() const { return m_d3d12_resource; }
        
        BufferType GetBufferType() const { return m_buffer_type; }
        BufferUsage GetBufferUsage() const { return m_buffer_usage; }
        uint32_t GetSize() const { return m_size; }
        uint32_t GetStride() const { return m_stride; }
        uint32_t GetElementCount() const { return m_element_count; }

        std::string m_name;
        BufferType m_buffer_type;
        BufferUsage m_buffer_usage;

    private:
        ID3D11Buffer* m_d3d_buffer = nullptr;
        ID3D11ShaderResourceView* m_shader_resource_view = nullptr;
        ID3D11UnorderedAccessView* m_unordered_access_view = nullptr;
        ID3D12Resource* m_d3d12_resource = nullptr;

        uint32_t m_size = 0;
        uint32_t m_stride = 0;
        uint32_t m_element_count = 0;
        bool m_is_mapped = false;
    }; // class Buffer
} // namespace Dolas

#endif // DOLAS_BUFFER_H

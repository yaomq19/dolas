#include "render/dolas_buffer.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include <iostream>
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
    Buffer::Buffer()
    {
        m_buffer_type = BufferType::VERTEX_BUFFER;
        m_buffer_usage = BufferUsage::DEFAULT;
    }

    Buffer::~Buffer()
    {
        Release();
    }

    bool Buffer::CreateBuffer(BufferType type, BufferUsage usage, uint32_t size, uint32_t stride, const void* initial_data)
    {
        ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
        if (!device)
        {
            LOG_ERROR("Buffer::CreateBuffer: D3D11 device is null");
            return false;
        }

        if (size == 0)
        {
            LOG_ERROR("Buffer::CreateBuffer: Buffer size cannot be zero");
            return false;
        }

        m_buffer_type = type;
        m_buffer_usage = usage;
        m_size = size;
        m_stride = stride;

        // 对于结构化缓冲区，计算元素数量
        if (type == BufferType::STRUCTURED_BUFFER && stride > 0)
        {
            m_element_count = size / stride;
        }

        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = size;
        desc.Usage = ConvertToD3DUsage(usage);
        desc.BindFlags = ConvertToD3DBindFlags(type, usage);
        desc.CPUAccessFlags = ConvertToD3DAccessFlags(usage);
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        // 设置结构化缓冲区的特殊属性
        if (type == BufferType::STRUCTURED_BUFFER)
        {
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.StructureByteStride = stride;
        }
        else if (type == BufferType::APPEND_CONSUME_BUFFER)
        {
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.StructureByteStride = stride;
        }

        D3D11_SUBRESOURCE_DATA init_data = {};
        D3D11_SUBRESOURCE_DATA* p_init_data = nullptr;
        
        if (initial_data)
        {
            init_data.pSysMem = initial_data;
            init_data.SysMemPitch = 0;
            init_data.SysMemSlicePitch = 0;
            p_init_data = &init_data;
        }

        HRESULT hr = device->CreateBuffer(&desc, p_init_data, &m_d3d_buffer);
        if (FAILED(hr))
        {
            LOG_ERROR("Buffer::CreateBuffer: Failed to create buffer");
            return false;
        }

        // 为结构化缓冲区创建Shader Resource View
        if (type == BufferType::STRUCTURED_BUFFER && (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.Format = DXGI_FORMAT_UNKNOWN;
            srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srv_desc.Buffer.FirstElement = 0;
            srv_desc.Buffer.NumElements = m_element_count;

            hr = device->CreateShaderResourceView(m_d3d_buffer, &srv_desc, &m_shader_resource_view);
            if (FAILED(hr))
            {
                LOG_ERROR("Buffer::CreateBuffer: Failed to create shader resource view");
            }
        }

        // 为结构化缓冲区创建Unordered Access View（如果需要）
        if (type == BufferType::STRUCTURED_BUFFER && (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS))
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
            uav_desc.Format = DXGI_FORMAT_UNKNOWN;
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uav_desc.Buffer.FirstElement = 0;
            uav_desc.Buffer.NumElements = m_element_count;
            uav_desc.Buffer.Flags = 0;

            if (type == BufferType::APPEND_CONSUME_BUFFER)
            {
                uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
            }

            hr = device->CreateUnorderedAccessView(m_d3d_buffer, &uav_desc, &m_unordered_access_view);
            if (FAILED(hr))
            {
                LOG_ERROR("Buffer::CreateBuffer: Failed to create unordered access view");
            }
        }

        LOG_INFO("Buffer::CreateBuffer: Successfully created buffer of size {0} bytes", size);
        return true;
    }

    bool Buffer::CreateVertexBuffer(uint32_t size, const void* initial_data, BufferUsage usage)
    {
        return CreateBuffer(BufferType::VERTEX_BUFFER, usage, size, 0, initial_data);
    }

    bool Buffer::CreateIndexBuffer(uint32_t size, const void* initial_data, BufferUsage usage)
    {
        return CreateBuffer(BufferType::INDEX_BUFFER, usage, size, 0, initial_data);
    }

    bool Buffer::CreateConstantBuffer(uint32_t size, const void* initial_data, BufferUsage usage)
    {
        // 常量缓冲区大小必须是16字节的倍数
        uint32_t aligned_size = (size + 15) & ~15;
        return CreateBuffer(BufferType::CONSTANT_BUFFER, usage, aligned_size, 0, initial_data);
    }

    bool Buffer::CreateStructuredBuffer(uint32_t element_count, uint32_t element_size, const void* initial_data, BufferUsage usage)
    {
        uint32_t total_size = element_count * element_size;
        return CreateBuffer(BufferType::STRUCTURED_BUFFER, usage, total_size, element_size, initial_data);
    }

    bool Buffer::UpdateData(const void* data, uint32_t size, uint32_t offset)
    {
        if (!m_d3d_buffer || !data)
        {
            LOG_ERROR("Buffer::UpdateData: Invalid buffer or data");
            return false;
        }

        if (offset + size > m_size)
        {
            LOG_ERROR("Buffer::UpdateData: Data size exceeds buffer bounds");
            return false;
        }

        ID3D11DeviceContext* context = g_dolas_engine.m_rhi->m_d3d_immediate_context;
        if (!context)
        {
            LOG_ERROR("Buffer::UpdateData: D3D11 device context is null");
            return false;
        }

        if (m_buffer_usage == BufferUsage::DYNAMIC)
        {
            // 对于动态缓冲区，使用Map/Unmap
            D3D11_MAPPED_SUBRESOURCE mapped_resource;
            HRESULT hr = context->Map(m_d3d_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            if (FAILED(hr))
            {
                LOG_ERROR("Buffer::UpdateData: Failed to map buffer");
                return false;
            }

            memcpy(static_cast<char*>(mapped_resource.pData) + offset, data, size);
            context->Unmap(m_d3d_buffer, 0);
        }
        else if (m_buffer_usage == BufferUsage::DEFAULT)
        {
            // 对于默认缓冲区，使用UpdateSubresource
            D3D11_BOX dest_box = {};
            dest_box.left = offset;
            dest_box.right = offset + size;
            dest_box.top = 0;
            dest_box.bottom = 1;
            dest_box.front = 0;
            dest_box.back = 1;

            context->UpdateSubresource(m_d3d_buffer, 0, &dest_box, data, 0, 0);
        }
        else
        {
            LOG_ERROR("Buffer::UpdateData: Buffer usage does not support updates");
            return false;
        }

        return true;
    }

    bool Buffer::Map(void** mapped_data, BufferAccess access)
    {
        if (!m_d3d_buffer || m_is_mapped)
        {
            LOG_ERROR("Buffer::Map: Invalid buffer or buffer already mapped");
            return false;
        }

        if (m_buffer_usage != BufferUsage::DYNAMIC && m_buffer_usage != BufferUsage::STAGING)
        {
            LOG_ERROR("Buffer::Map: Buffer usage does not support mapping");
            return false;
        }

        ID3D11DeviceContext* context = g_dolas_engine.m_rhi->m_d3d_immediate_context;
        if (!context)
        {
            LOG_ERROR("Buffer::Map: D3D11 device context is null");
            return false;
        }

        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        D3D11_MAP map_type = ConvertToD3DMapType(access);

        HRESULT hr = context->Map(m_d3d_buffer, 0, map_type, 0, &mapped_resource);
        if (FAILED(hr))
        {
            LOG_ERROR("Buffer::Map: Failed to map buffer");
            return false;
        }

        *mapped_data = mapped_resource.pData;
        m_is_mapped = true;
        return true;
    }

    void Buffer::Unmap()
    {
        if (!m_d3d_buffer || !m_is_mapped)
        {
            return;
        }

        ID3D11DeviceContext* context = g_dolas_engine.m_rhi->m_d3d_immediate_context;
        if (context)
        {
            context->Unmap(m_d3d_buffer, 0);
        }

        m_is_mapped = false;
    }

    bool Buffer::ReadData(void* output_data, uint32_t size, uint32_t offset)
    {
        if (!m_d3d_buffer || !output_data)
        {
            LOG_ERROR("Buffer::ReadData: Invalid buffer or output data");
            return false;
        }

        if (m_buffer_usage != BufferUsage::STAGING)
        {
            LOG_ERROR("Buffer::ReadData: Buffer must be staging buffer for reading");
            return false;
        }

        if (offset + size > m_size)
        {
            LOG_ERROR("Buffer::ReadData: Read size exceeds buffer bounds");
            return false;
        }

        void* mapped_data;
        if (!Map(&mapped_data, BufferAccess::READ))
        {
            return false;
        }

        memcpy(output_data, static_cast<const char*>(mapped_data) + offset, size);
        Unmap();

        return true;
    }

    D3D11_USAGE Buffer::ConvertToD3DUsage(BufferUsage usage)
    {
        switch (usage)
        {
            case BufferUsage::DEFAULT:   return D3D11_USAGE_DEFAULT;
            case BufferUsage::IMMUTABLE: return D3D11_USAGE_IMMUTABLE;
            case BufferUsage::DYNAMIC:   return D3D11_USAGE_DYNAMIC;
            case BufferUsage::STAGING:   return D3D11_USAGE_STAGING;
            default:                     return D3D11_USAGE_DEFAULT;
        }
    }

    UINT Buffer::ConvertToD3DAccessFlags(BufferUsage usage)
    {
        switch (usage)
        {
            case BufferUsage::DYNAMIC:   return D3D11_CPU_ACCESS_WRITE;
            case BufferUsage::STAGING:   return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            default:                     return 0;
        }
    }

    UINT Buffer::ConvertToD3DBindFlags(BufferType type, BufferUsage usage)
    {
        UINT bind_flags = 0;

        switch (type)
        {
            case BufferType::VERTEX_BUFFER:
                bind_flags = D3D11_BIND_VERTEX_BUFFER;
                break;
            case BufferType::INDEX_BUFFER:
                bind_flags = D3D11_BIND_INDEX_BUFFER;
                break;
            case BufferType::CONSTANT_BUFFER:
                bind_flags = D3D11_BIND_CONSTANT_BUFFER;
                break;
            case BufferType::STRUCTURED_BUFFER:
                bind_flags = D3D11_BIND_SHADER_RESOURCE;
                if (usage == BufferUsage::DEFAULT)
                {
                    bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
                }
                break;
            case BufferType::APPEND_CONSUME_BUFFER:
                bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
                break;
            case BufferType::STAGING_BUFFER:
                bind_flags = 0; // Staging buffers cannot be bound to pipeline
                break;
        }

        return bind_flags;
    }

    D3D11_MAP Buffer::ConvertToD3DMapType(BufferAccess access)
    {
        if (m_buffer_usage == BufferUsage::DYNAMIC)
        {
            return D3D11_MAP_WRITE_DISCARD;
        }
        else if (m_buffer_usage == BufferUsage::STAGING)
        {
            switch (access)
            {
                case BufferAccess::READ:       return D3D11_MAP_READ;
                case BufferAccess::WRITE:      return D3D11_MAP_WRITE;
                case BufferAccess::READ_WRITE: return D3D11_MAP_READ_WRITE;
                default:                       return D3D11_MAP_READ_WRITE;
            }
        }
        
        return D3D11_MAP_WRITE_DISCARD;
    }

    void Buffer::Release()
    {
        if (m_is_mapped)
        {
            Unmap();
        }

        if (m_shader_resource_view)  { m_shader_resource_view->Release();  m_shader_resource_view = nullptr; }
        if (m_unordered_access_view) { m_unordered_access_view->Release(); m_unordered_access_view = nullptr; }
        if (m_d3d_buffer)                { m_d3d_buffer->Release();                m_d3d_buffer = nullptr; }

        m_size = 0;
        m_stride = 0;
        m_element_count = 0;
        m_is_mapped = false;
    }

} // namespace Dolas

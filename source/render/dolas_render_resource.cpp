#include "render/dolas_render_resource.h"
#include "render/dolas_texture.h"
#include "render/dolas_buffer.h"
#include <iostream>

namespace Dolas
{
    RenderResource::RenderResource()
    {

    }

    RenderResource::~RenderResource()
    {

    }

    bool RenderResource::Initialize()
    {
        return true;
    }

    bool RenderResource::Clear()
    {
        // 清理所有纹理资源
        for (auto& pair : m_textures)
        {
            if (pair.second)
            {
                pair.second->Release();
            }
        }
        m_textures.clear();

        // 清理所有缓冲区资源
        for (auto& pair : m_buffers)
        {
            if (pair.second)
            {
                pair.second->Release();
            }
        }
        m_buffers.clear();

        std::cout << "RenderResource::Clear: All resources cleared for " << m_name << std::endl;
        return true;
    }

    bool RenderResource::AddTexture(const std::string& name, std::shared_ptr<Texture> texture)
    {
        if (!texture)
        {
            std::cerr << "RenderResource::AddTexture: Texture is null for name: " << name << std::endl;
            return false;
        }

        if (m_textures.find(name) != m_textures.end())
        {
            std::cerr << "RenderResource::AddTexture: Texture with name '" << name << "' already exists" << std::endl;
            return false;
        }

        m_textures[name] = texture;
        std::cout << "RenderResource::AddTexture: Added texture: " << name << std::endl;
        return true;
    }

    std::shared_ptr<Texture> RenderResource::GetTexture(const std::string& name) const
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool RenderResource::HasTexture(const std::string& name) const
    {
        return m_textures.find(name) != m_textures.end();
    }

    bool RenderResource::RemoveTexture(const std::string& name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            if (it->second)
            {
                it->second->Release();
            }
            m_textures.erase(it);
            std::cout << "RenderResource::RemoveTexture: Removed texture: " << name << std::endl;
            return true;
        }
        
        std::cerr << "RenderResource::RemoveTexture: Texture not found: " << name << std::endl;
        return false;
    }

    bool RenderResource::AddBuffer(const std::string& name, std::shared_ptr<Buffer> buffer)
    {
        if (!buffer)
        {
            std::cerr << "RenderResource::AddBuffer: Buffer is null for name: " << name << std::endl;
            return false;
        }

        if (m_buffers.find(name) != m_buffers.end())
        {
            std::cerr << "RenderResource::AddBuffer: Buffer with name '" << name << "' already exists" << std::endl;
            return false;
        }

        m_buffers[name] = buffer;
        std::cout << "RenderResource::AddBuffer: Added buffer: " << name << std::endl;
        return true;
    }

    std::shared_ptr<Buffer> RenderResource::GetBuffer(const std::string& name) const
    {
        auto it = m_buffers.find(name);
        if (it != m_buffers.end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool RenderResource::HasBuffer(const std::string& name) const
    {
        return m_buffers.find(name) != m_buffers.end();
    }

    bool RenderResource::RemoveBuffer(const std::string& name)
    {
        auto it = m_buffers.find(name);
        if (it != m_buffers.end())
        {
            if (it->second)
            {
                it->second->Release();
            }
            m_buffers.erase(it);
            std::cout << "RenderResource::RemoveBuffer: Removed buffer: " << name << std::endl;
            return true;
        }
        
        std::cerr << "RenderResource::RemoveBuffer: Buffer not found: " << name << std::endl;
        return false;
    }

    void RenderResource::AddTextures(const std::vector<std::pair<std::string, std::shared_ptr<Texture>>>& textures)
    {
        for (const auto& pair : textures)
        {
            AddTexture(pair.first, pair.second);
        }
    }

    void RenderResource::AddBuffers(const std::vector<std::pair<std::string, std::shared_ptr<Buffer>>>& buffers)
    {
        for (const auto& pair : buffers)
        {
            AddBuffer(pair.first, pair.second);
        }
    }

    uint32_t RenderResource::GetTotalTextureMemory() const
    {
        uint32_t total_memory = 0;
        for (const auto& pair : m_textures)
        {
            if (pair.second)
            {
                // 估算纹理内存使用（宽 * 高 * 4字节，假设RGBA格式）
                total_memory += pair.second->GetWidth() * pair.second->GetHeight() * 4;
            }
        }
        return total_memory;
    }

    uint32_t RenderResource::GetTotalBufferMemory() const
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

    std::vector<std::string> RenderResource::GetTextureNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_textures.size());
        for (const auto& pair : m_textures)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    std::vector<std::string> RenderResource::GetBufferNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_buffers.size());
        for (const auto& pair : m_buffers)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    std::vector<std::shared_ptr<Texture>> RenderResource::GetAllTextures() const
    {
        std::vector<std::shared_ptr<Texture>> textures;
        textures.reserve(m_textures.size());
        for (const auto& pair : m_textures)
        {
            textures.push_back(pair.second);
        }
        return textures;
    }

    std::vector<std::shared_ptr<Buffer>> RenderResource::GetAllBuffers() const
    {
        std::vector<std::shared_ptr<Buffer>> buffers;
        buffers.reserve(m_buffers.size());
        for (const auto& pair : m_buffers)
        {
            buffers.push_back(pair.second);
        }
        return buffers;
    }

    std::shared_ptr<Texture> RenderResource::GetRenderTarget(const std::string& name) const
    {
        return GetTexture(name);
    }

    std::shared_ptr<Texture> RenderResource::GetDepthStencil(const std::string& name) const
    {
        return GetTexture(name);
    }

    std::shared_ptr<Buffer> RenderResource::GetConstantBuffer(const std::string& name) const
    {
        auto buffer = GetBuffer(name);
        if (buffer && buffer->GetBufferType() == BufferType::CONSTANT_BUFFER)
        {
            return buffer;
        }
        return nullptr;
    }

    std::shared_ptr<Buffer> RenderResource::GetVertexBuffer(const std::string& name) const
    {
        auto buffer = GetBuffer(name);
        if (buffer && buffer->GetBufferType() == BufferType::VERTEX_BUFFER)
        {
            return buffer;
        }
        return nullptr;
    }

    std::shared_ptr<Buffer> RenderResource::GetIndexBuffer(const std::string& name) const
    {
        auto buffer = GetBuffer(name);
        if (buffer && buffer->GetBufferType() == BufferType::INDEX_BUFFER)
        {
            return buffer;
        }
        return nullptr;
    }

} // namespace Dolas 
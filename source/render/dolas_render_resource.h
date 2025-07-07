#ifndef DOLAS_RENDER_RESOURCE_H
#define DOLAS_RENDER_RESOURCE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace Dolas
{
    class Texture;
    class Buffer;

    class RenderResource
    {
        friend class RenderResourceManager;
    public:
        RenderResource();
        ~RenderResource();

        bool Initialize();
        bool Clear();

        // Texture资源管理
        bool AddTexture(const std::string& name, std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> GetTexture(const std::string& name) const;
        bool HasTexture(const std::string& name) const;
        bool RemoveTexture(const std::string& name);
        
        // Buffer资源管理
        bool AddBuffer(const std::string& name, std::shared_ptr<Buffer> buffer);
        std::shared_ptr<Buffer> GetBuffer(const std::string& name) const;
        bool HasBuffer(const std::string& name) const;
        bool RemoveBuffer(const std::string& name);

        // 批量资源操作
        void AddTextures(const std::vector<std::pair<std::string, std::shared_ptr<Texture>>>& textures);
        void AddBuffers(const std::vector<std::pair<std::string, std::shared_ptr<Buffer>>>& buffers);

        // 资源统计
        size_t GetTextureCount() const { return m_textures.size(); }
        size_t GetBufferCount() const { return m_buffers.size(); }
        uint32_t GetTotalTextureMemory() const;
        uint32_t GetTotalBufferMemory() const;

        // 资源列表获取
        std::vector<std::string> GetTextureNames() const;
        std::vector<std::string> GetBufferNames() const;
        std::vector<std::shared_ptr<Texture>> GetAllTextures() const;
        std::vector<std::shared_ptr<Buffer>> GetAllBuffers() const;

        // 常用资源快速访问
        std::shared_ptr<Texture> GetRenderTarget(const std::string& name = "main_rt") const;
        std::shared_ptr<Texture> GetDepthStencil(const std::string& name = "main_ds") const;
        std::shared_ptr<Buffer> GetConstantBuffer(const std::string& name) const;
        std::shared_ptr<Buffer> GetVertexBuffer(const std::string& name) const;
        std::shared_ptr<Buffer> GetIndexBuffer(const std::string& name) const;

        std::string m_name;

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
        std::unordered_map<std::string, std::shared_ptr<Buffer>> m_buffers;
    }; // class RenderResource
} // namespace Dolas

#endif // DOLAS_RENDER_RESOURCE_H 
#ifndef DOLAS_TEXTURE_MANAGER_H
#define DOLAS_TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_texture.h"

namespace Dolas
{
    class TextureManager
    {
    public:
        TextureManager();
        ~TextureManager();
        bool Initialize();
        bool Clear();

        std::shared_ptr<Texture> GetOrCreateTexture(const std::string& file_name, bool generate_mips = true);
        std::shared_ptr<Texture> CreateTexture(const std::string& file_name, bool generate_mips = true);
        
        // 创建自定义纹理
        std::shared_ptr<Texture> CreateTexture(const std::string& name, uint32_t width, uint32_t height, TextureFormat format, const void* initial_data = nullptr);
        std::shared_ptr<Texture> CreateRenderTarget(const std::string& name, uint32_t width, uint32_t height, TextureFormat format);
        std::shared_ptr<Texture> CreateDepthStencil(const std::string& name, uint32_t width, uint32_t height);
        
        // 从内存加载纹理
        std::shared_ptr<Texture> CreateTextureFromMemory(const std::string& name, const void* data, size_t size, bool generate_mips = true);

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    }; // class TextureManager
} // namespace Dolas

#endif // DOLAS_TEXTURE_MANAGER_H

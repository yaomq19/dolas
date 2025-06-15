#include "manager/dolas_texture_manager.h"
#include "render/dolas_texture.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include <iostream>
#include <fstream>

namespace Dolas
{
    TextureManager::TextureManager()
    {

    }

    TextureManager::~TextureManager()
    {

    }
    
    bool TextureManager::Initialize()
    {
        return true;
    }

    bool TextureManager::Clear()
    {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it)
        {
            std::shared_ptr<Texture> texture = it->second;
            if (texture)
            {
                texture->Release();
            }
        }
        m_textures.clear();
        return true;
    }

    std::shared_ptr<Texture> TextureManager::GetOrCreateTexture(const std::string& file_name, bool generate_mips)
    {
        if (m_textures.find(file_name) == m_textures.end())
        {
            std::shared_ptr<Texture> texture = CreateTexture(file_name, generate_mips);
            if (texture != nullptr)
            {
                m_textures[file_name] = texture;
            }
            return texture;
        }
        return m_textures[file_name];
    }

    std::shared_ptr<Texture> TextureManager::CreateTexture(const std::string& file_name, bool generate_mips)
    {
        std::string texture_path = PathUtils::GetContentDir() + "texture/" + file_name;

        // 检查文件是否存在
        std::ifstream file(texture_path);
        if (!file.is_open())
        {
            std::cerr << "TextureManager::CreateTexture: file is not found in " << texture_path << std::endl;
            return nullptr;
        }
        file.close();

        // 创建纹理对象
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        
        //// 加载纹理
        //if (!texture->LoadFromFile(texture_path, generate_mips))
        //{
        //    std::cerr << "TextureManager::CreateTexture: Failed to load texture from " << texture_path << std::endl;
        //    return nullptr;
        //}

        std::cout << "TextureManager::CreateTexture: Successfully created texture from " << texture_path << std::endl;
        return texture;
    }

    std::shared_ptr<Texture> TextureManager::CreateTexture(const std::string& name, uint32_t width, uint32_t height, TextureFormat format, const void* initial_data)
    {
        // 检查是否已存在
        if (m_textures.find(name) != m_textures.end())
        {
            return m_textures[name];
        }

        // 创建纹理对象
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        
        // 创建纹理
        if (!texture->CreateTexture(width, height, format, initial_data))
        {
            std::cerr << "TextureManager::CreateTexture: Failed to create custom texture: " << name << std::endl;
            return nullptr;
        }

        m_textures[name] = texture;
        std::cout << "TextureManager::CreateTexture: Successfully created custom texture: " << name << " (" << width << "x" << height << ")" << std::endl;
        return texture;
    }

    std::shared_ptr<Texture> TextureManager::CreateRenderTarget(const std::string& name, uint32_t width, uint32_t height, TextureFormat format)
    {
        // 检查是否已存在
        if (m_textures.find(name) != m_textures.end())
        {
            return m_textures[name];
        }

        // 创建纹理对象
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        
        // 创建渲染目标
        if (!texture->CreateRenderTarget(width, height, format))
        {
            std::cerr << "TextureManager::CreateRenderTarget: Failed to create render target: " << name << std::endl;
            return nullptr;
        }

        m_textures[name] = texture;
        std::cout << "TextureManager::CreateRenderTarget: Successfully created render target: " << name << " (" << width << "x" << height << ")" << std::endl;
        return texture;
    }

    std::shared_ptr<Texture> TextureManager::CreateDepthStencil(const std::string& name, uint32_t width, uint32_t height)
    {
        // 检查是否已存在
        if (m_textures.find(name) != m_textures.end())
        {
            return m_textures[name];
        }

        // 创建纹理对象
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        
        // 创建深度模板缓冲
        if (!texture->CreateDepthStencil(width, height))
        {
            std::cerr << "TextureManager::CreateDepthStencil: Failed to create depth stencil: " << name << std::endl;
            return nullptr;
        }

        m_textures[name] = texture;
        std::cout << "TextureManager::CreateDepthStencil: Successfully created depth stencil: " << name << " (" << width << "x" << height << ")" << std::endl;
        return texture;
    }

    std::shared_ptr<Texture> TextureManager::CreateTextureFromMemory(const std::string& name, const void* data, size_t size, bool generate_mips)
    {
        // 检查是否已存在
        if (m_textures.find(name) != m_textures.end())
        {
            return m_textures[name];
        }

        // 创建纹理对象
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        
        //// 从内存加载纹理
        //if (!texture->LoadFromMemory(data, size, generate_mips))
        //{
        //    std::cerr << "TextureManager::CreateTextureFromMemory: Failed to load texture from memory: " << name << std::endl;
        //    return nullptr;
        //}

        m_textures[name] = texture;
        std::cout << "TextureManager::CreateTextureFromMemory: Successfully created texture from memory: " << name << std::endl;
        return texture;
    }

} // namespace Dolas

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
            Texture* texture = it->second;
            if (texture)
            {
                texture->Release();
            }
        }
        m_textures.clear();
        return true;
    }

    TextureID TextureManager::CreateTexture(const std::string& file_name)
    {
        std::string texture_file_path = PathUtils::GetTextureDir() + file_name;

        // todo : load texture from file

        Texture* texture = DOLAS_NEW(Texture);
        texture->m_file_id = STRING_ID(texture_file_path);
        m_textures[texture->m_file_id] = texture;
        return texture->m_file_id;
    }
    
    Texture* TextureManager::GetTexture(TextureID texture_id)
    {
        if (m_textures.find(texture_id) == m_textures.end())
        {
            return nullptr;
        }
        return m_textures[texture_id];
    }

} // namespace Dolas

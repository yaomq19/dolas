#ifndef DOLAS_TEXTURE_MANAGER_H
#define DOLAS_TEXTURE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_texture.h"
#include "base/dolas_base.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    class TextureManager
    {
    public:
        TextureManager();
        ~TextureManager();
        bool Initialize();
        bool Clear();

        TextureID CreateTexture(const std::string& file_name);
        Texture* GetTexture(TextureID texture_id);
    private:
        std::unordered_map<TextureID, Texture*> m_textures;
    }; // class TextureManager
} // namespace Dolas

#endif // DOLAS_TEXTURE_MANAGER_H

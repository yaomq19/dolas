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

        TextureID CreateTextureFromFile(const std::string& file_name);
        TextureID CreateTexture();
        Texture* GetTexture(TextureID texture_id);
        DXGI_FORMAT ConvertToDXGIFormat(TextureFormat format);

    private:
        std::unordered_map<TextureID, Texture*> m_textures;
    }; // class TextureManager
} // namespace Dolas

#endif // DOLAS_TEXTURE_MANAGER_H

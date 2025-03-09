#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H
#include <d3d11.h>

namespace Dolas
{
    class TextureManager
    {
    public:
        TextureManager();
        ~TextureManager();
        bool Initialize();
        ID3D11Texture2D* GetBackBuffer() { return m_back_buffer; }
    private:
        ID3D11Texture2D* m_back_buffer = nullptr;
    };
}

#endif // TEXTURE_MANAGER_H
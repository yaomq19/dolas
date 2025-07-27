#ifndef DOLAS_MATERIAL_H
#define DOLAS_MATERIAL_H

#include <string>
#include <unordered_map>
#include <d3d11.h>
#include "common/dolas_hash.h"

namespace Dolas
{
    class Material
    {
        friend class MaterialManager;
    public:
        Material();
        ~Material();
        class VertexShader* GetVertexShader();
        class PixelShader* GetPixelShader();
        
    protected:
        MaterialID m_file_id;
        class VertexShader* m_vertex_shader = nullptr;
        class PixelShader* m_pixel_shader = nullptr;
        std::unordered_map<int, TextureID> m_textures;
    }; // class Material
} // namespace Dolas

#endif // DOLAS_MATERIAL_H
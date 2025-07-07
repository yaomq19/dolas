#ifndef DOLAS_MATERIAL_H
#define DOLAS_MATERIAL_H

#include <string>
#include <vector>
#include <d3d11.h>

namespace Dolas
{
    class Material
    {
        friend class MaterialManager;
    public:
        Material();
        ~Material();

        std::string m_file_path;
        ID3D11VertexShader* m_vertex_shader = nullptr;
        ID3D11PixelShader* m_pixel_shader = nullptr;
        std::vector<std::pair<int, std::string>> m_textures;
    }; // class Material
} // namespace Dolas

#endif // DOLAS_MATERIAL_H
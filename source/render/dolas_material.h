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
        bool BuildFromFile(const std::string& file_path);
        std::string GetFilePath();
        class VertexShader* GetVertexShader();
        class PixelShader* GetPixelShader();
        std::vector<std::pair<int, std::string>> GetTextures();
        
    protected:
        std::string m_file_path;
        class VertexShader* m_vertex_shader = nullptr;
        class PixelShader* m_pixel_shader = nullptr;
        std::vector<std::pair<int, std::string>> m_textures;
    }; // class Material
} // namespace Dolas

#endif // DOLAS_MATERIAL_H
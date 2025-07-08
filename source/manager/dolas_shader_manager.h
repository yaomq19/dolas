#ifndef DOLAS_SHADER_MANAGER_H
#define DOLAS_SHADER_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "render/dolas_shader.h"

namespace Dolas
{
    class ShaderManager
    {
    public:
        ShaderManager();
        ~ShaderManager();
        bool Initialize();
        bool Clear();
        VertexShader* GetOrCreateVertexShader(const std::string& file_path, const std::string& entry_point);
        PixelShader* GetOrCreatePixelShader(const std::string& file_path, const std::string& entry_point);
    private:
        VertexShader* CreateVertexShader(const std::string& file_path, const std::string& entry_point);
        PixelShader* CreatePixelShader(const std::string& file_path, const std::string& entry_point);
        std::string GenerateShaderKey(const std::string& file_path, const std::string& entry_point);

        std::unordered_map<std::string, VertexShader*> m_vertex_shaders;
        std::unordered_map<std::string, PixelShader*> m_pixel_shaders;
    }; // class ShaderManager
} // namespace Dolas

#endif // DOLAS_SHADER_MANAGER_H

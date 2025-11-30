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
        VertexContext* GetOrCreateVertexShader(const std::string& file_path, const std::string& entry_point);
        PixelContext* GetOrCreatePixelShader(const std::string& file_path, const std::string& entry_point);
        void dumpShaderReflectionInfos() const;
    private:
        VertexContext* CreateVertexShader(const std::string& file_path, const std::string& entry_point);
        PixelContext* CreatePixelShader(const std::string& file_path, const std::string& entry_point);
        std::string GenerateShaderKey(const std::string& file_path, const std::string& entry_point);

        std::unordered_map<std::string, VertexContext*> m_vertex_shaders;
        std::unordered_map<std::string, PixelContext*> m_pixel_shaders;
    }; // class ShaderManager
} // namespace Dolas

#endif // DOLAS_SHADER_MANAGER_H

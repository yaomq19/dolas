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

        std::shared_ptr<Shader> GetOrCreateShader(const std::string& file_name, ShaderType type, const std::string& entry_point = "main");
        std::shared_ptr<Shader> CreateShader(const std::string& file_name, ShaderType type, const std::string& entry_point = "main");
        
        // 直接从内存加载着色器
        std::shared_ptr<Shader> CreateShaderFromMemory(const std::string& name, const void* data, size_t size, ShaderType type, const std::string& entry_point = "main");

    private:
        std::string GenerateShaderKey(const std::string& file_name, ShaderType type, const std::string& entry_point);
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    }; // class ShaderManager
} // namespace Dolas

#endif // DOLAS_SHADER_MANAGER_H

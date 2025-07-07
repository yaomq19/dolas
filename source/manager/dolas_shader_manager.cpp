#include "manager/dolas_shader_manager.h"
#include "render/dolas_shader.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include <iostream>
#include <fstream>

namespace Dolas
{
    ShaderManager::ShaderManager()
    {

    }

    ShaderManager::~ShaderManager()
    {

    }
    
    bool ShaderManager::Initialize()
    {
        return true;
    }

    bool ShaderManager::Clear()
    {
        for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
        {
            std::shared_ptr<Shader> shader = it->second;
            if (shader)
            {
                shader->Release();
            }
        }
        m_shaders.clear();
        return true;
    }

    std::shared_ptr<Shader> ShaderManager::GetOrCreateShader(const std::string& file_name, ShaderType type, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_name, type, entry_point);
        
        if (m_shaders.find(key) == m_shaders.end())
        {
            std::shared_ptr<Shader> shader = CreateShader(file_name, type, entry_point);
            if (shader != nullptr)
            {
                m_shaders[key] = shader;
            }
            return shader;
        }
        return m_shaders[key];
    }

    std::shared_ptr<Shader> ShaderManager::CreateShader(const std::string& file_name, ShaderType type, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetShadersSourceDir() + file_name;

        // 创建着色器对象
        std::shared_ptr<Shader> shader = std::make_shared<Shader>();
        
        // 加载着色器
        if (!shader->LoadFromFile(shader_path, type, entry_point))
        {
            std::cerr << "ShaderManager::CreateShader: Failed to load shader from " << shader_path << std::endl;
            return nullptr;
        }

        std::cout << "ShaderManager::CreateShader: Successfully created shader from " << shader_path << std::endl;
        return shader;
    }

    std::string ShaderManager::GenerateShaderKey(const std::string& file_name, ShaderType type, const std::string& entry_point)
    {
        std::string type_str;
        switch (type)
        {
            case ShaderType::VERTEX_SHADER:   type_str = "VS"; break;
            case ShaderType::PIXEL_SHADER:    type_str = "PS"; break;
            case ShaderType::GEOMETRY_SHADER: type_str = "GS"; break;
            case ShaderType::HULL_SHADER:     type_str = "HS"; break;
            case ShaderType::DOMAIN_SHADER:   type_str = "DS"; break;
            case ShaderType::COMPUTE_SHADER:  type_str = "CS"; break;
            default: type_str = "UNKNOWN"; break;
        }
        
        return file_name + "_" + type_str + "_" + entry_point;
    }

} // namespace Dolas

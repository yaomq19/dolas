#include "manager/dolas_shader_manager.h"
#include "render/dolas_shader.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "base/dolas_dx_trace.h"
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
        for (auto it = m_vertex_shaders.begin(); it != m_vertex_shaders.end(); ++it)
        {
            VertexShader* vertex_shader = it->second;
            if (vertex_shader)
            {
                vertex_shader->Release();
            }
        }
        m_vertex_shaders.clear();

        for (auto it = m_pixel_shaders.begin(); it != m_pixel_shaders.end(); ++it)
        {
            PixelShader* pixel_shader = it->second;
            if (pixel_shader)
            {
                pixel_shader->Release();
            }
        }
        m_pixel_shaders.clear();
        return true;
    }

    VertexShader* ShaderManager::GetOrCreateVertexShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_path, entry_point);
        
        if (m_vertex_shaders.find(key) == m_vertex_shaders.end())
        {
            VertexShader* vertex_shader = CreateVertexShader(file_path, entry_point);
            if (vertex_shader != nullptr)
            {
                m_vertex_shaders[key] = vertex_shader;
            }

            return vertex_shader;
        }
        return m_vertex_shaders[key];
    }

    PixelShader* ShaderManager::GetOrCreatePixelShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_path, entry_point);
        if (m_pixel_shaders.find(key) == m_pixel_shaders.end())
        {
            PixelShader* pixel_shader = CreatePixelShader(file_path, entry_point);
            if (pixel_shader != nullptr)
            {
                m_pixel_shaders[key] = pixel_shader;
            }

            return pixel_shader;
        }
        return m_pixel_shaders[key];
    }
    
    VertexShader* ShaderManager::CreateVertexShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetShadersSourceDir() + file_path;

        // 创建着色器对象
        VertexShader* vertex_shader = DOLAS_NEW(VertexShader);
        
        // 加载着色器
        if (!vertex_shader->BuildFromFile(shader_path, entry_point))
        {
            std::cerr << "ShaderManager::CreateShader: Failed to load shader from " << shader_path << std::endl;
            vertex_shader->Release();
            DOLAS_DELETE(vertex_shader);
            return nullptr;
        }

        std::cout << "ShaderManager::CreateShader: Successfully created shader from " << shader_path << std::endl;
        return vertex_shader;
    }

    PixelShader* ShaderManager::CreatePixelShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetShadersSourceDir() + file_path;

        PixelShader* pixel_shader = DOLAS_NEW(PixelShader);
        if (!pixel_shader->BuildFromFile(shader_path, entry_point))
        {
            std::cerr << "ShaderManager::CreateShader: Failed to load shader from " << shader_path << std::endl;
            pixel_shader->Release();
            DOLAS_DELETE(pixel_shader);
            return nullptr;
        }

        std::cout << "ShaderManager::CreateShader: Successfully created shader from " << shader_path << std::endl;
        return pixel_shader;
    }

    std::string ShaderManager::GenerateShaderKey(const std::string& file_path, const std::string& entry_point)
    {
        return file_path + "_" + entry_point;
        // return file_path + "_" + entry_point + "_" + std::to_string(std::hash<std::string>{}(file_path + "_" + entry_point));
    }

} // namespace Dolas

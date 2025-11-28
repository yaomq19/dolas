#include "manager/dolas_shader_manager.h"
#include "render/dolas_shader.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "base/dolas_dx_trace.h"
#include <iostream>
#include <fstream>
#include "manager/dolas_log_system_manager.h"
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
            VertexContext* vertex_context = it->second;
            if (vertex_context)
            {
                vertex_context->Release();
            }
        }
        m_vertex_shaders.clear();

        for (auto it = m_pixel_shaders.begin(); it != m_pixel_shaders.end(); ++it)
        {
            PixelContext* pixel_context = it->second;
            if (pixel_context)
            {
                pixel_context->Release();
            }
        }
        m_pixel_shaders.clear();
        return true;
    }

    VertexContext* ShaderManager::GetOrCreateVertexShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_path, entry_point);
        
        if (m_vertex_shaders.find(key) == m_vertex_shaders.end())
        {
            VertexContext* vertex_context = CreateVertexShader(file_path, entry_point);
            if (vertex_context != nullptr)
            {
                m_vertex_shaders[key] = vertex_context;
            }

            return vertex_context;
        }
        return m_vertex_shaders[key];
    }

    PixelContext* ShaderManager::GetOrCreatePixelShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_path, entry_point);
        if (m_pixel_shaders.find(key) == m_pixel_shaders.end())
        {
            PixelContext* pixel_context = CreatePixelShader(file_path, entry_point);
            if (pixel_context != nullptr)
            {
                m_pixel_shaders[key] = pixel_context;
            }

            return pixel_context;
        }
        return m_pixel_shaders[key];
    }
    
    VertexContext* ShaderManager::CreateVertexShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetShadersSourceDir() + file_path;

        // 创建着色器对象
        VertexContext* vertex_context = DOLAS_NEW(VertexContext);
        
        // 加载着色器
        if (!vertex_context->BuildFromFile(shader_path, entry_point))
        {
            LOG_ERROR("Failed to load shader from {0}", shader_path);
            vertex_context->Release();
            DOLAS_DELETE(vertex_context);
            return nullptr;
        }

        LOG_INFO("Successfully created shader from {0}", shader_path);
        return vertex_context;
    }

    PixelContext* ShaderManager::CreatePixelShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetShadersSourceDir() + file_path;

        PixelContext* pixel_context = DOLAS_NEW(PixelContext);
        if (!pixel_context->BuildFromFile(shader_path, entry_point))
        {
            LOG_ERROR("Failed to load shader from {0}", shader_path);
            pixel_context->Release();
            DOLAS_DELETE(pixel_context);
            return nullptr;
        }

        LOG_INFO("Successfully created shader from {0}", shader_path);
        return pixel_context;
    }

    std::string ShaderManager::GenerateShaderKey(const std::string& file_path, const std::string& entry_point)
    {
        return file_path + "_" + entry_point;
        // return file_path + "_" + entry_point + "_" + std::to_string(std::hash<std::string>{}(file_path + "_" + entry_point));
    }

} // namespace Dolas

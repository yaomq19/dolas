#include "manager/dolas_shader_manager.h"
#include "render/dolas_shader.h"
#include "dolas_base.h"
#include "dolas_paths.h"
#include "render/dolas_dx_trace.h"
#include <iostream>
#include <fstream>
#include "dolas_log_system_manager.h"
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
        for (auto& it : m_vertex_shaders)
        {
            VertexShader* vertex_shader = it.second;
            if (vertex_shader)
            {
                vertex_shader->Release();
                DOLAS_DELETE(vertex_shader);
            }
        }
        m_vertex_shaders.clear();

        for (auto& it : m_pixel_shaders)
        {
            PixelShader* pixel_shader = it.second;
            if (pixel_shader)
            {
                pixel_shader->Release();
                DOLAS_DELETE(pixel_shader);
            }
        }
        m_pixel_shaders.clear();
        return true;
    }

	std::shared_ptr<VertexContext> ShaderManager::GetOrCreateVertexContext(const std::string& file_path, const std::string& entry_point)
	{
        std::string key = GenerateShaderKey(file_path, entry_point);
        
        VertexShader* vertex_shader = nullptr;
        auto it = m_vertex_shaders.find(key);
        if (it == m_vertex_shaders.end())
        {
            // 动态类型为 VertexShader，函数返回类型是 VertexContext* 以保持兼容
            VertexContext* created = CreateVertexShader(file_path, entry_point);
            if (!created)
            {
                return nullptr;
            }
            vertex_shader = static_cast<VertexShader*>(created);
            m_vertex_shaders[key] = vertex_shader;
        }
        else
        {
            vertex_shader = it->second;
        }

        // 返回一个不负责销毁的 shared_ptr，生命周期由 ShaderManager 管理
        return std::shared_ptr<VertexContext>(vertex_shader, [](VertexContext*) {});
    }

    std::shared_ptr<PixelContext> ShaderManager::GetOrCreatePixelContext(const std::string& file_path, const std::string& entry_point)
    {
        std::string key = GenerateShaderKey(file_path, entry_point);

        PixelShader* pixel_shader = nullptr;
        auto it = m_pixel_shaders.find(key);
        if (it == m_pixel_shaders.end())
        {
            PixelContext* created = CreatePixelShader(file_path, entry_point);
            if (!created)
            {
                return nullptr;
            }
            pixel_shader = static_cast<PixelShader*>(created);
            m_pixel_shaders[key] = pixel_shader;
        }
        else
        {
            pixel_shader = it->second;
        }

        return std::shared_ptr<PixelContext>(pixel_shader, [](PixelContext*) {});
    }
    
    void ShaderManager::dumpShaderReflectionInfos() const
    {
        LOG_INFO("/*****************************************/");
        LOG_INFO("Dump Vertex Shaders...");
        for (auto vertex_shader_iter : m_vertex_shaders)
        {
            const std::string& shader_name = vertex_shader_iter.first;
            const ShaderContext* shader_ptr = vertex_shader_iter.second;
            LOG_INFO("-----------------------");
            LOG_INFO("shader name: {0}", shader_name);
            shader_ptr->dumpShaderReflectionInfo();
            LOG_INFO("-----------------------");
		}

        LOG_INFO("Dump Pixel Shaders...");
        for (auto pixel_shader_iter : m_pixel_shaders)
        {
            const std::string& shader_name = pixel_shader_iter.first;
            const ShaderContext* shader_ptr = pixel_shader_iter.second;
            LOG_INFO("-----------------------");
            LOG_INFO("shader name: {0}", shader_name);
            shader_ptr->dumpShaderReflectionInfo();
            LOG_INFO("-----------------------");
        }
        LOG_INFO("/*****************************************/");
    }

    VertexContext* ShaderManager::CreateVertexShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetEngineContentDir() + file_path;

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

        vertex_context->PostBuildFromFile();
        LOG_INFO("Successfully created shader from {0}", shader_path);
        return vertex_context;
    }

    PixelContext* ShaderManager::CreatePixelShader(const std::string& file_path, const std::string& entry_point)
    {
        std::string shader_path = PathUtils::GetEngineContentDir() + file_path;

        PixelContext* pixel_context = DOLAS_NEW(PixelContext);
        if (!pixel_context->BuildFromFile(shader_path, entry_point))
        {
            LOG_ERROR("Failed to load shader from {0}", shader_path);
            pixel_context->Release();
            DOLAS_DELETE(pixel_context);
            return nullptr;
        }

        pixel_context->PostBuildFromFile();
        LOG_INFO("Successfully created shader from {0}", shader_path);
        return pixel_context;
    }

    std::string ShaderManager::GenerateShaderKey(const std::string& file_path, const std::string& entry_point)
    {
        return file_path + "_" + entry_point;
        // return file_path + "_" + entry_point + "_" + std::to_string(std::hash<std::string>{}(file_path + "_" + entry_point));
    }

} // namespace Dolas

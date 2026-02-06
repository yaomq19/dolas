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

        // 返回可共享的 Vertex/Pixel 上下文，内部通过缓存避免重复创建底层 D3D shader
        std::shared_ptr<VertexContext> GetOrCreateVertexContext(const std::string& file_path, const std::string& entry_point);
        std::shared_ptr<PixelContext>  GetOrCreatePixelContext(const std::string& file_path, const std::string& entry_point);
        void                           dumpShaderReflectionInfos() const;
    private:
        VertexContext* CreateVertexShader(const std::string& file_path, const std::string& entry_point);
        PixelContext*  CreatePixelShader(const std::string& file_path, const std::string& entry_point);
        std::string    GenerateShaderKey(const std::string& file_path, const std::string& entry_point);

        // ShaderManager 持有共享的底层着色器对象（VertexShader / PixelShader）
        std::unordered_map<std::string, VertexShader*> m_vertex_shaders;
        std::unordered_map<std::string, PixelShader*>  m_pixel_shaders;
    }; // class ShaderManager
} // namespace Dolas

#endif // DOLAS_SHADER_MANAGER_H

#ifndef DOLAS_SHADER_H
#define DOLAS_SHADER_H

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace Dolas
{
    enum class ShaderType
    {
        VERTEX_SHADER,
        PIXEL_SHADER,
        GEOMETRY_SHADER,
        HULL_SHADER,
        DOMAIN_SHADER,
        COMPUTE_SHADER
    };

    class Shader
    {
        friend class ShaderManager;
    public:
        Shader();
        ~Shader();

        bool LoadFromFile(const std::string& file_path, ShaderType type, const std::string& entry_point = "main");
        bool LoadFromMemory(const void* data, size_t size, ShaderType type, const std::string& entry_point = "main");
        void Release();

        // Getters
        ID3D11VertexShader* GetVertexShader() const { return m_vertex_shader; }
        ID3D11PixelShader* GetPixelShader() const { return m_pixel_shader; }
        ID3D11GeometryShader* GetGeometryShader() const { return m_geometry_shader; }
        ID3D11HullShader* GetHullShader() const { return m_hull_shader; }
        ID3D11DomainShader* GetDomainShader() const { return m_domain_shader; }
        ID3D11ComputeShader* GetComputeShader() const { return m_compute_shader; }
        ID3DBlob* GetShaderBlob() const { return m_shader_blob; }
        ShaderType GetShaderType() const { return m_shader_type; }

        std::string m_file_path;
        std::string m_entry_point;
        ShaderType m_shader_type;

    private:
        bool CompileShader(const std::string& source_code, const std::string& target, const std::string& entry_point);

        ID3D11VertexShader* m_vertex_shader = nullptr;
        ID3D11PixelShader* m_pixel_shader = nullptr;
        ID3D11GeometryShader* m_geometry_shader = nullptr;
        ID3D11HullShader* m_hull_shader = nullptr;
        ID3D11DomainShader* m_domain_shader = nullptr;
        ID3D11ComputeShader* m_compute_shader = nullptr;
        ID3DBlob* m_shader_blob = nullptr;
        ID3DBlob* m_error_blob = nullptr;
    }; // class Shader
} // namespace Dolas

#endif // DOLAS_SHADER_H

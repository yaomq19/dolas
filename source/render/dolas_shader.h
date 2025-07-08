#ifndef DOLAS_SHADER_H
#define DOLAS_SHADER_H

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace Dolas
{
    class Shader
    {
        friend class ShaderManager;
    public:
        Shader();
        ~Shader();

        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) = 0;
        virtual void Release();
        virtual ID3DBlob* GetD3DShaderBlob();
        virtual ID3D11ShaderReflection* GetD3DShaderReflection();
    protected:
        virtual void GenerateReflectionAndDesc();

        std::string m_file_path;
        std::string m_entry_point;

        ID3DBlob* m_d3d_shader_blob = nullptr;
        ID3D11ShaderReflection* m_d3d_shader_reflection = nullptr;
        D3D11_SHADER_DESC m_shader_desc;
    }; // class Shader

    class VertexShader : public Shader
    {
    public:
        VertexShader();
        ~VertexShader();
        bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        void Release() override;
        ID3D11VertexShader* GetD3DVertexShader();
    protected:
        ID3D11VertexShader* m_d3d_vertex_shader = nullptr;
    }; // class VertexShader

    class PixelShader : public Shader
    {
    public:
        PixelShader();
        ~PixelShader();
        bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        void Release() override;
        ID3D11PixelShader* GetD3DPixelShader();
    protected:
        ID3D11PixelShader* m_d3d_pixel_shader = nullptr;
    }; // class PixelShader
    
} // namespace Dolas

#endif // DOLAS_SHADER_H

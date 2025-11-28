#ifndef DOLAS_SHADER_H
#define DOLAS_SHADER_H

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include "core/dolas_rhi.h"
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

        virtual void Bind(DolasRHI* rhi, ID3D11ClassInstance* const* class_instances, UINT num_class_instances, const std::unordered_map<int, TextureID>& temp_vertex_shader_textures) = 0;

        void SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv);
        void SetShaderResourceView(size_t slot, TextureID texture_id);
        void SetShaderResourceView(size_t slot, class Texture* texture);
        ID3D11ShaderResourceView* GetShaderResourceView(size_t slot);

    protected:
        virtual void GenerateReflectionAndDesc();

        std::string m_file_path;
        std::string m_entry_point;

        ID3DBlob* m_d3d_shader_blob = nullptr;
        ID3D11ShaderReflection* m_d3d_shader_reflection = nullptr;
        D3D11_SHADER_DESC m_shader_desc;

        std::unordered_map<size_t, ID3D11ShaderResourceView*> m_shader_resource_views;

    }; // class Shader

    class VertexShader : public Shader
    {
    public:
        VertexShader();
        ~VertexShader();
        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        virtual void Release() override;
        ID3D11VertexShader* GetD3DVertexShader();

        virtual void Bind(DolasRHI* rhi, ID3D11ClassInstance* const* class_instances, UINT num_class_instances, const std::unordered_map<int, TextureID>& temp_vertex_shader_textures) override;

    protected:
        ID3D11VertexShader* m_d3d_vertex_shader = nullptr;
    }; // class VertexShader

    class PixelShader : public Shader
    {
    public:
        PixelShader();
        ~PixelShader();
        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        virtual void Release() override;
        ID3D11PixelShader* GetD3DPixelShader();
        virtual void Bind(DolasRHI* rhi, ID3D11ClassInstance* const* class_instances, UINT num_class_instances, const std::unordered_map<int, TextureID>& temp_pixel_shader_textures) override;
        
    protected:
        ID3D11PixelShader* m_d3d_pixel_shader = nullptr;
    }; // class PixelShader
    
} // namespace Dolas

#endif // DOLAS_SHADER_H

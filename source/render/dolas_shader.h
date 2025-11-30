#ifndef DOLAS_SHADER_H
#define DOLAS_SHADER_H

#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include "core/dolas_rhi.h"
namespace Dolas
{
    struct ConstantBufferInfo
    {
		D3D11_SHADER_BUFFER_DESC constant_buffer_desc;
		std::vector<D3D11_SHADER_VARIABLE_DESC> variable_descs;
    };

    struct ShaderReflectionInfo
    {
        D3D11_SHADER_DESC shader_desc;
		std::vector<ConstantBufferInfo> constant_buffer_descs;
    };

    class ShaderContext
    {
        friend class ShaderManager;
    public:
        ShaderContext();
        ~ShaderContext();

        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) = 0;
        virtual void Release();
        virtual ID3DBlob* GetD3DShaderBlob();
        void SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv);
        void SetShaderResourceView(size_t slot, TextureID texture_id);
        void SetShaderResourceView(size_t slot, class Texture* texture);
        const std::unordered_map<size_t, ID3D11ShaderResourceView*>& GetSlotToSRVMap() const {return m_slot_to_srv_map;};
        const ShaderReflectionInfo& GetShaderReflectionInfo() const {return m_shader_reflection_info;};
        void dumpShaderReflectionInfo() const;
		ID3D11Buffer* GetGlobalConstantBuffer() { return m_global_constant_buffer; };
        void ConvertTextureIDMapToSRVMap();
        // Global constant buffer data（已根据反射布局预打包好的原始字节）
        const std::vector<uint8_t>& GetGlobalConstantBufferData() const { return m_global_cb_data; }
        // 设置某个全局变量（按变量名写入 GlobalConstants cbuffer 对应区域）
        void SetGlobalVariable(const std::string& name, const std::vector<float>& values);
    protected:
        void AnalyzeConstantBuffers(UINT constant_buffers_count);
        void GenerateReflectionAndDesc();
        void CreateGlobalConstantBuffer();
        void PostBuildFromFile();
    protected:
        std::string m_file_path;
        std::string m_entry_point;

        ID3DBlob* m_d3d_shader_blob = nullptr;
        ID3D11ShaderReflection* m_d3d_shader_reflection = nullptr;

		ShaderReflectionInfo m_shader_reflection_info;

        std::unordered_map<size_t, TextureID> m_slot_to_texture_map;
        std::unordered_map<size_t, ID3D11ShaderResourceView*> m_slot_to_srv_map;

		ID3D11Buffer* m_global_constant_buffer = nullptr;
        std::vector<uint8_t> m_global_cb_data;

    }; // class ShaderContext

    class VertexContext : public ShaderContext
    {
    public:
        VertexContext();
        ~VertexContext();
        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        virtual void Release() override;
        ID3D11VertexShader* GetD3DVertexShader();

    protected:
        ID3D11VertexShader* m_d3d_vertex_shader = nullptr;
    }; // class VertexContext

    class PixelContext : public ShaderContext
    {
    public:
        PixelContext();
        ~PixelContext();
        virtual bool BuildFromFile(const std::string& file_path, const std::string& entry_point) override;
        virtual void Release() override;
        ID3D11PixelShader* GetD3DPixelShader();
        
    protected:
        ID3D11PixelShader* m_d3d_pixel_shader = nullptr;
    }; // class PixelContext
    
} // namespace Dolas

#endif // DOLAS_SHADER_H

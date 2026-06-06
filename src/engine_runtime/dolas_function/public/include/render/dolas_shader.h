#ifndef DOLAS_SHADER_H
#define DOLAS_SHADER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <d3d12.h>
#include "dolas_hash.h"
#include "dolas_math.h"
#include "render/dolas_rhi_common.h"

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11ShaderReflection;
struct ID3D11ShaderResourceView;
struct ID3D11Buffer;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D12Resource;

namespace Dolas
{
    struct ConstantBufferVariableInfo
    {
        std::string name;
        UInt start_offset = 0;
        UInt size = 0;
        UInt flags = 0;
    };

    struct ConstantBufferInfo
    {
        std::string name;
        UInt size = 0;
        UInt variable_count = 0;
		std::vector<ConstantBufferVariableInfo> variable_descs;
    };

    struct ShaderReflectionInfo
    {
        UInt constant_buffer_count = 0;
        UInt bound_resource_count = 0;
        UInt instruction_count = 0;
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
        ShaderBytecodeView GetShaderBytecode() const;
        void SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv);
        void SetShaderResourceView(size_t slot, TextureID texture_id);
        void SetShaderResourceView(size_t slot, class Texture* texture);
        const std::unordered_map<size_t, ID3D11ShaderResourceView*>& GetSlotToSRVMap() const {return m_slot_to_srv_map;};
        const std::unordered_map<size_t, TextureID>& GetSlotToTextureMap() const { return m_slot_to_texture_map; }
        const std::unordered_map<size_t, D3D12_CPU_DESCRIPTOR_HANDLE>& GetSlotToD3D12SRVCpuMap() const { return m_slot_to_d3d12_srv_cpu_map; }
        const std::unordered_map<size_t, D3D12_GPU_DESCRIPTOR_HANDLE>& GetSlotToD3D12SRVMap() const { return m_slot_to_d3d12_srv_map; }
        const ShaderReflectionInfo& GetShaderReflectionInfo() const {return m_shader_reflection_info;};
        void dumpShaderReflectionInfo() const;
		ID3D11Buffer* GetGlobalConstantBuffer() { return m_global_constant_buffer; };
		ID3D12Resource* GetD3D12GlobalConstantBuffer() { return m_d3d12_global_constant_buffer; };
        void ConvertTextureIDMapToSRVMap();
        // Global constant buffer data（已根据反射布局预打包好的原始字节）
        const std::vector<uint8_t>& GetGlobalConstantBufferData() const { return m_global_cb_data; }
        // 设置某个全局变量（按变量名写入 GlobalConstants cbuffer 对应区域）
        void SetGlobalVariable(const std::string& name, const Vector4& values);
    protected:
        void AnalyzeConstantBuffers(UInt constant_buffers_count);
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
        std::unordered_map<size_t, D3D12_CPU_DESCRIPTOR_HANDLE> m_slot_to_d3d12_srv_cpu_map;
        std::unordered_map<size_t, D3D12_GPU_DESCRIPTOR_HANDLE> m_slot_to_d3d12_srv_map;

		ID3D11Buffer* m_global_constant_buffer = nullptr;
        ID3D12Resource* m_d3d12_global_constant_buffer = nullptr;
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

    // Lightweight shader objects used for caching compiled shaders in ShaderManager.
    // Currently they are thin aliases of the corresponding *Context types,
    // which already own ID3D11*Shader, reflection info and GlobalConstantBuffer.
    class VertexShader : public VertexContext
    {
    public:
        using VertexContext::VertexContext;
    };

    class PixelShader : public PixelContext
    {
    public:
        using PixelContext::PixelContext;
    };
    
} // namespace Dolas

#endif // DOLAS_SHADER_H

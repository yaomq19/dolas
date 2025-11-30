#include <iostream>
#include <fstream>
#include <sstream>
#include <d3dcompiler.h>

#include "render/dolas_shader.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "base/dolas_dx_trace.h"
#include "base/dolas_string_util.h"
#include "base/dolas_dx_trace.h"
#include "manager/dolas_texture_manager.h"
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
    ShaderContext::ShaderContext()
    {
    }

    ShaderContext::~ShaderContext()
    {
    
    }

	void ShaderContext::Release()
	{
		if (m_d3d_shader_blob)
		{
			m_d3d_shader_blob->Release();
			m_d3d_shader_blob = nullptr;
		}
		if (m_d3d_shader_reflection)
		{
			m_d3d_shader_reflection->Release();
			m_d3d_shader_reflection = nullptr;
		}
	}

	ID3DBlob* ShaderContext::GetD3DShaderBlob()
	{
		return m_d3d_shader_blob;
	}

	void ShaderContext::AnalyzeConstantBuffers(UINT constant_buffers_count)
	{
		for (UINT cb_index = 0; cb_index < constant_buffers_count; ++cb_index)
		{
			// ͨ��������ȡ��������������ӿ�
			ID3D11ShaderReflectionConstantBuffer* pConstantBuffer =
				m_d3d_shader_reflection->GetConstantBufferByIndex(cb_index);

			if (pConstantBuffer == nullptr)
			{
				LOG_ERROR("�����޷���ȡ���������� {0} �ķ���ӿ�\n", cb_index);
				continue;
			}

			// ������������������
			ConstantBufferInfo cb_info;

			D3D11_SHADER_BUFFER_DESC cb_desc;
			pConstantBuffer->GetDesc(&cb_desc);
			cb_info.constant_buffer_desc = cb_desc;
			
			for (UINT variable_index = 0; variable_index < cb_desc.Variables; ++variable_index)
			{
				ID3D11ShaderReflectionVariable* pVariable =
					pConstantBuffer->GetVariableByIndex(variable_index);
				if (pVariable == nullptr)
				{
					LOG_ERROR("�����޷���ȡ���������� {0} �б��� {1} �ķ���ӿ�\n", cb_index, variable_index);
					continue;
				}
				D3D11_SHADER_VARIABLE_DESC var_desc;
				pVariable->GetDesc(&var_desc);
				cb_info.variable_descs.push_back(var_desc);
			}

			m_shader_reflection_info.constant_buffer_descs.push_back(cb_info);
		}
	}

	void ShaderContext::GenerateReflectionAndDesc()
	{
		HR(D3DReflect(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_d3d_shader_reflection));
		
		HR(m_d3d_shader_reflection->GetDesc(&m_shader_reflection_info.shader_desc));

		AnalyzeConstantBuffers(m_shader_reflection_info.shader_desc.ConstantBuffers);

		// HR(m_d3d_shader_reflection->GetConstant(&m_shader_desc));
		// HR(m_d3d_shader_reflection->GetDesc(&m_shader_desc));
	}

	void ShaderContext::CreateGlobalConstantBuffer()
	{
		// 先释放旧的（如果有）
		if (m_global_constant_buffer)
		{
			m_global_constant_buffer->Release();
			m_global_constant_buffer = nullptr;
		}

		// 在反射出来的 CB 里查找名为 "GlobalConstants" 的 cbuffer
		const char* kGlobalCBName = "GlobalConstants";
		const ConstantBufferInfo* target_cb_info = nullptr;

		for (const auto& cb_info : m_shader_reflection_info.constant_buffer_descs)
		{
			const D3D11_SHADER_BUFFER_DESC& cb_desc = cb_info.constant_buffer_desc;
			const char* cb_name = cb_desc.Name ? cb_desc.Name : "";
			if (std::strcmp(cb_name, kGlobalCBName) == 0)
			{
				target_cb_info = &cb_info;
				break;
			}
		}

		if (!target_cb_info)
		{
			// 当前 shader 没有 GlobalConstants，就什么都不做
			return;
		}

		const D3D11_SHADER_BUFFER_DESC& cb_desc = target_cb_info->constant_buffer_desc;

        // 初始化 CPU 端缓存区，用来存放 GlobalConstants 的原始字节数据
        m_global_cb_data.clear();
        m_global_cb_data.resize(cb_desc.Size);

		// 创建对应大小的常量缓冲区（CPU 可写，GPU 读取）
		D3D11_BUFFER_DESC cbd = {};
		cbd.ByteWidth = cb_desc.Size; // HLSL 编译器保证是 16 字节对齐
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0;

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
		D3D11_SUBRESOURCE_DATA init_data = {};
		init_data.pSysMem = nullptr; // 先不填初始值，后面通过 Map/Unmap 更新

		HR(device->CreateBuffer(&cbd, nullptr, &m_global_constant_buffer));

		LOG_INFO("Created GlobalConstants CB for shader {0}, size = {1} bytes", m_file_path, cb_desc.Size);
	}

	void ShaderContext::PostBuildFromFile()
	{
		GenerateReflectionAndDesc();
		CreateGlobalConstantBuffer();
	}

	void ShaderContext::SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv)
	{
		DOLAS_RETURN_IF_NULL(srv);
		m_slot_to_srv_map[slot] = srv;
	}

	void ShaderContext::SetShaderResourceView(size_t slot, TextureID texture_id)
	{
		Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		// 材质阶段通常只关心 TextureID，这里也顺便填充 SRV 映射，方便后续直接绑定
		m_slot_to_texture_map[slot] = texture_id;
		m_slot_to_srv_map[slot] = srv;
	}

	void ShaderContext::SetShaderResourceView(size_t slot, Texture* texture)
	{
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		m_slot_to_srv_map[slot] = srv;
	}

	void ShaderContext::dumpShaderReflectionInfo() const
	{
		const auto& shader_desc = m_shader_reflection_info.shader_desc;

		LOG_INFO("file = {0}, entry = {1}", m_file_path, m_entry_point);
		LOG_INFO("  ConstantBuffers = {0}, BoundResources = {1}, Instructions = {2}",
			shader_desc.ConstantBuffers,
			shader_desc.BoundResources,
			shader_desc.InstructionCount);

		for (size_t cb_index = 0; cb_index < m_shader_reflection_info.constant_buffer_descs.size(); ++cb_index)
		{
			const auto& cb_info = m_shader_reflection_info.constant_buffer_descs[cb_index];
			const auto& cb_desc = cb_info.constant_buffer_desc;
			
			const char* cb_name = cb_desc.Name ? cb_desc.Name : "";
			LOG_INFO("name = {0}, size = {1} bytes, variables = {2}",
				cb_name,
				cb_desc.Size,
				cb_desc.Variables);

			for (size_t var_index = 0; var_index < cb_info.variable_descs.size(); ++var_index)
			{
				const auto& var_desc = cb_info.variable_descs[var_index];
				const char* var_name = var_desc.Name ? var_desc.Name : "";

				LOG_INFO("    Var[{0}]: name = {1}, startOffset = {2}, size = {3} bytes, flags = {4}",
					var_index,
					var_name,
					var_desc.StartOffset,
					var_desc.Size,
					static_cast<UInt>(var_desc.uFlags));
			}
		}
	}

	void ShaderContext::ConvertTextureIDMapToSRVMap()
	{
        if (m_slot_to_texture_map.empty())
        {
            return;
        }

        TextureManager* tex_mgr = g_dolas_engine.m_texture_manager;
        DOLAS_RETURN_IF_NULL(tex_mgr);

        for (auto& pair : m_slot_to_texture_map)
        {
            size_t slot = pair.first;
            TextureID tex_id = pair.second;

            Texture* tex = tex_mgr->GetTextureByTextureID(tex_id);
            DOLAS_CONTINUE_IF_NULL(tex);

            ID3D11ShaderResourceView* srv = tex->GetShaderResourceView();
            DOLAS_CONTINUE_IF_NULL(srv);

            m_slot_to_srv_map[slot] = srv;
        }
	}

    void ShaderContext::SetGlobalVariable(const std::string& name, const std::vector<float>& values)
    {
        if (m_global_cb_data.empty())
        {
            // 当前 shader 没有 GlobalConstants，忽略
            return;
        }

        // 找到 GlobalConstants 中同名变量的布局信息
        const char* kGlobalCBName = "GlobalConstants";
        const ConstantBufferInfo* target_cb_info = nullptr;
        for (const auto& cb_info : m_shader_reflection_info.constant_buffer_descs)
        {
            const D3D11_SHADER_BUFFER_DESC& cb_desc = cb_info.constant_buffer_desc;
            const char* cb_name = cb_desc.Name ? cb_desc.Name : "";
            if (std::strcmp(cb_name, kGlobalCBName) == 0)
            {
                target_cb_info = &cb_info;
                break;
            }
        }
        if (!target_cb_info)
        {
            return;
        }

        for (const auto& var_desc : target_cb_info->variable_descs)
        {
            const char* var_name_cstr = var_desc.Name ? var_desc.Name : "";
            if (name != var_name_cstr)
            {
                continue;
            }

            // 计算要拷贝的字节数（不超过变量大小）
            size_t src_bytes = values.size() * sizeof(float);
            size_t copy_bytes = std::min<size_t>(src_bytes, var_desc.Size);

            if (copy_bytes == 0)
            {
                return;
            }

            // 写入到 CPU 端 Global CB 缓冲区中相应的偏移位置
            uint8_t* dst = m_global_cb_data.data() + var_desc.StartOffset;
            std::memcpy(dst, values.data(), copy_bytes);
            return;
        }
	}

	VertexContext::VertexContext()
	{

	}

	VertexContext::~VertexContext()
	{

	}

	bool VertexContext::BuildFromFile(const std::string& file_path, const std::string& entry_point)
	{
		m_entry_point = entry_point;
		m_file_path = file_path;
		ID3DBlob* error_blob = nullptr;
		HR(D3DCompileFromFile(
			StringUtil::StringToWString(file_path).c_str(), // file path
			nullptr, // macros
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // include?
			entry_point.c_str(), // entry point
			"vs_5_0", // shader model
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // flags
			0, // effect flags
			&m_d3d_shader_blob, // shader blob
			&error_blob)); // error blob

		if (error_blob)
		{
			LOG_ERROR(static_cast<char*>(error_blob->GetBufferPointer()));
			error_blob->Release();
			error_blob = nullptr;
			return false;
		}

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
		HR(device->CreateVertexShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_vertex_shader));

		return true;
	}

	void VertexContext::Release()
	{
		ShaderContext::Release();
		if (m_d3d_vertex_shader)
		{
			m_d3d_vertex_shader->Release();
			m_d3d_vertex_shader = nullptr;
		}
	}

	ID3D11VertexShader* VertexContext::GetD3DVertexShader()
	{
		return m_d3d_vertex_shader;
	}

	PixelContext::PixelContext()
	{

	}

	PixelContext::~PixelContext()
	{

	}

	Bool PixelContext::BuildFromFile(const std::string& file_path, const std::string& entry_point)
	{
		m_entry_point = entry_point;
		m_file_path = file_path;
		ID3DBlob* error_blob = nullptr;
		HR(D3DCompileFromFile(
			StringUtil::StringToWString(file_path).c_str(), // file path
			nullptr, // macros
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // include
			entry_point.c_str(), // entry point
			"ps_5_0", // shader model
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // flags
			0, // effect flags
			&m_d3d_shader_blob, // shader blob
			&error_blob)); // error blob

		if (error_blob)
		{
			LOG_ERROR(static_cast<char*>(error_blob->GetBufferPointer()));
			error_blob->Release();
			error_blob = nullptr;
			return false;
		}

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
		HR(device->CreatePixelShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_pixel_shader));

		return true;
	}

	void PixelContext::Release()
	{
		ShaderContext::Release();
		if (m_d3d_pixel_shader)
		{
			m_d3d_pixel_shader->Release();
			m_d3d_pixel_shader = nullptr;
		}
	}

	ID3D11PixelShader* PixelContext::GetD3DPixelShader()
	{
		return m_d3d_pixel_shader;
	}

} // namespace Dolas

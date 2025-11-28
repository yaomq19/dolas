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
namespace Dolas
{
    Shader::Shader()
    {
    }

    Shader::~Shader()
    {
    
    }

	void Shader::Release()
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

	ID3DBlob* Shader::GetD3DShaderBlob()
	{
		return m_d3d_shader_blob;
	}

	ID3D11ShaderReflection* Shader::GetD3DShaderReflection()
	{
		return m_d3d_shader_reflection;
	}

	void Shader::GenerateReflectionAndDesc()
	{
		HR(D3DReflect(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_d3d_shader_reflection));
		m_d3d_shader_reflection->GetDesc(&m_shader_desc);
	}

	void Shader::SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv)
	{
		DOLAS_RETURN_IF_NULL(srv);
		m_shader_resource_views[slot] = srv;
	}

	void Shader::SetShaderResourceView(size_t slot, TextureID texture_id)
	{
		Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		m_shader_resource_views[slot] = srv;
	}

	void Shader::SetShaderResourceView(size_t slot, Texture* texture)
	{
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		m_shader_resource_views[slot] = srv;
	}

	ID3D11ShaderResourceView* Shader::GetShaderResourceView(size_t slot)
	{
		if (m_shader_resource_views.find(slot) == m_shader_resource_views.end())
		{
			return nullptr;
		}
		return m_shader_resource_views[slot];
	}

	VertexShader::VertexShader()
	{

	}

	VertexShader::~VertexShader()
	{

	}

	bool VertexShader::BuildFromFile(const std::string& file_path, const std::string& entry_point)
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
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
			error_blob->Release();
			error_blob = nullptr;
			return false;
		}

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
		HR(device->CreateVertexShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_vertex_shader));

		GenerateReflectionAndDesc();

		return true;
	}

	void VertexShader::Release()
	{
		Shader::Release();
		if (m_d3d_vertex_shader)
		{
			m_d3d_vertex_shader->Release();
			m_d3d_vertex_shader = nullptr;
		}
	}

	ID3D11VertexShader* VertexShader::GetD3DVertexShader()
	{
		return m_d3d_vertex_shader;
	}

	void VertexShader::Bind(DolasRHI* rhi, ID3D11ClassInstance* const* class_instances, UINT num_class_instances, const std::unordered_map<int, TextureID>& temp_vertex_shader_textures)
	{
		rhi->GetD3D11DeviceContext()->VSSetShader(m_d3d_vertex_shader, class_instances, num_class_instances);

		for (auto tex_slot_id_pair : temp_vertex_shader_textures)
		{
			int slot = tex_slot_id_pair.first;
			TextureID texture_id = tex_slot_id_pair.second;
			Texture* tex = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
			DOLAS_CONTINUE_IF_NULL(tex);
			ID3D11ShaderResourceView* srv = tex->GetShaderResourceView();
			DOLAS_CONTINUE_IF_NULL(srv);

			m_shader_resource_views[slot] = srv;
		}

		std::unordered_map<size_t, ID3D11ShaderResourceView*>::iterator iter = m_shader_resource_views.begin();
		for ( ;iter != m_shader_resource_views.end(); iter++)
		{
			rhi->GetD3D11DeviceContext()->VSSetShaderResources(iter->first, 1, &iter->second);
		}
	}

	PixelShader::PixelShader()
	{

	}

	PixelShader::~PixelShader()
	{

	}

	Bool PixelShader::BuildFromFile(const std::string& file_path, const std::string& entry_point)
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
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
			error_blob->Release();
			error_blob = nullptr;
			return false;
		}

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
		HR(device->CreatePixelShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_pixel_shader));

		GenerateReflectionAndDesc();

		return true;
	}

	void PixelShader::Release()
	{
		Shader::Release();
		if (m_d3d_pixel_shader)
		{
			m_d3d_pixel_shader->Release();
			m_d3d_pixel_shader = nullptr;
		}
	}

	void PixelShader::Bind(DolasRHI* rhi, ID3D11ClassInstance* const* class_instances, UINT num_class_instances, const std::unordered_map<int, TextureID>& temp_pixel_shader_textures)
	{
		rhi->GetD3D11DeviceContext()->PSSetShader(this->m_d3d_pixel_shader, class_instances, num_class_instances);

		for (auto tex_slot_id_pair : temp_pixel_shader_textures)
		{
			int slot = tex_slot_id_pair.first;
			TextureID texture_id = tex_slot_id_pair.second;
			Texture* tex = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
			DOLAS_CONTINUE_IF_NULL(tex);
			ID3D11ShaderResourceView* srv = tex->GetShaderResourceView();
			DOLAS_CONTINUE_IF_NULL(srv);

			m_shader_resource_views[slot] = srv;
		}

		std::unordered_map<size_t, ID3D11ShaderResourceView*>::iterator iter = m_shader_resource_views.begin();

		for (; iter != m_shader_resource_views.end(); iter++)
		{
			rhi->GetD3D11DeviceContext()->PSSetShaderResources(iter->first, 1, &iter->second);
		}
	}

	ID3D11PixelShader* PixelShader::GetD3DPixelShader()
	{
		return m_d3d_pixel_shader;
	}

} // namespace Dolas

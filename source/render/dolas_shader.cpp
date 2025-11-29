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

	void ShaderContext::GenerateReflectionAndDesc()
	{
		HR(D3DReflect(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_d3d_shader_reflection));
		
		HR(m_d3d_shader_reflection->GetDesc(&m_shader_reflection_info.shader_desc));
		// HR(m_d3d_shader_reflection->GetConstant(&m_shader_desc));
		// HR(m_d3d_shader_reflection->GetDesc(&m_shader_desc));
	}

	void ShaderContext::SetShaderResourceView(size_t slot, ID3D11ShaderResourceView* srv)
	{
		DOLAS_RETURN_IF_NULL(srv);
		m_shader_resource_views[slot] = srv;
	}

	void ShaderContext::SetShaderResourceView(size_t slot, TextureID texture_id)
	{
		Texture* texture = g_dolas_engine.m_texture_manager->GetTextureByTextureID(texture_id);
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		m_shader_resource_views[slot] = srv;
	}

	void ShaderContext::SetShaderResourceView(size_t slot, Texture* texture)
	{
		DOLAS_RETURN_IF_NULL(texture);

		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		DOLAS_RETURN_IF_NULL(srv);

		m_shader_resource_views[slot] = srv;
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
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
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
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
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

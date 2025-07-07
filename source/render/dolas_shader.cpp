#include <iostream>
#include <fstream>
#include <sstream>

#include "render/dolas_shader.h"
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "base/dolas_dx_trace.h"
#include "base/dolas_string_util.h"

namespace Dolas
{
    Shader::Shader()
    {
        m_shader_type = ShaderType::VERTEX_SHADER;
        m_entry_point = "main";
    }

    Shader::~Shader()
    {
        Release();
    }

    bool Shader::LoadFromFile(const std::string& file_path, ShaderType type, const std::string& entry_point)
    {
        m_file_path = file_path;
        m_shader_type = type;
        m_entry_point = entry_point;

        // 确定编译目标
        std::string target;
        switch (type)
        {
            case ShaderType::VERTEX_SHADER:   target = "vs_5_0"; break;
            case ShaderType::PIXEL_SHADER:    target = "ps_5_0"; break;
            case ShaderType::GEOMETRY_SHADER: target = "gs_5_0"; break;
            case ShaderType::HULL_SHADER:     target = "hs_5_0"; break;
            case ShaderType::DOMAIN_SHADER:   target = "ds_5_0"; break;
            case ShaderType::COMPUTE_SHADER:  target = "cs_5_0"; break;
            default:
                std::cerr << "Shader::LoadFromFile: Unsupported shader type" << std::endl;
                return false;
        }

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		// 编译着色器
		std::wstring wfile_path = StringUtil::StringToWString(file_path);  // 避免临时对象
		HRESULT hr = D3DCompileFromFile(
			wfile_path.c_str(),
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entry_point.c_str(),  // 使用传入的入口点
			target.c_str(),       // 使用计算的目标
			flags,
			0,
			&m_shader_blob,
			&m_error_blob
		);

		if (FAILED(hr))
		{
			if (m_error_blob != nullptr)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(m_error_blob->GetBufferPointer()));
			}
			if (m_error_blob)
			{
				m_error_blob->Release();
				m_error_blob = nullptr;
			}
			return false;  // 修复：返回bool类型
		}

		// 创建对应类型的着色器对象
		ID3D11Device* device = g_dolas_engine.m_rhi->m_d3d_device;
		if (!device)
		{
			std::cerr << "Shader::CompileShader: D3D11 device is null" << std::endl;
			return false;
		}

		switch (m_shader_type)
		{
		case ShaderType::VERTEX_SHADER:
			hr = device->CreateVertexShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_vertex_shader
			);
			break;

		case ShaderType::PIXEL_SHADER:
			hr = device->CreatePixelShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_pixel_shader
			);
			break;

		case ShaderType::GEOMETRY_SHADER:
			hr = device->CreateGeometryShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_geometry_shader
			);
			break;

		case ShaderType::HULL_SHADER:
			hr = device->CreateHullShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_hull_shader
			);
			break;

		case ShaderType::DOMAIN_SHADER:
			hr = device->CreateDomainShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_domain_shader
			);
			break;

		case ShaderType::COMPUTE_SHADER:
			hr = device->CreateComputeShader(
				m_shader_blob->GetBufferPointer(),
				m_shader_blob->GetBufferSize(),
				nullptr,
				&m_compute_shader
			);
			break;
		}

		if (FAILED(hr))
		{
			std::cerr << "Shader::CompileShader: Failed to create shader object" << std::endl;
			return false;
		}

		std::cout << "Shader::CompileShader: Successfully compiled " << target << " shader" << std::endl;
		return true;
    }

    void Shader::Release()
    {
        if (m_vertex_shader)   { m_vertex_shader->Release();   m_vertex_shader = nullptr; }
        if (m_pixel_shader)    { m_pixel_shader->Release();    m_pixel_shader = nullptr; }
        if (m_geometry_shader) { m_geometry_shader->Release(); m_geometry_shader = nullptr; }
        if (m_hull_shader)     { m_hull_shader->Release();     m_hull_shader = nullptr; }
        if (m_domain_shader)   { m_domain_shader->Release();   m_domain_shader = nullptr; }
        if (m_compute_shader)  { m_compute_shader->Release();  m_compute_shader = nullptr; }
        if (m_shader_blob)     { m_shader_blob->Release();     m_shader_blob = nullptr; }
        if (m_error_blob)      { m_error_blob->Release();      m_error_blob = nullptr; }
    }

} // namespace Dolas

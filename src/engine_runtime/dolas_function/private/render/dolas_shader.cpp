#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <d3d11.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "render/dolas_shader.h"
#include "dolas_engine.h"
#include "dolas_render_hardware_interface.h"
#include "render/dolas_rhi.h"
#include "render/dolas_dx_trace.h"
#include "dolas_string_util.h"
#include "render/dolas_dx_trace.h"
#include "dolas_paths.h"
#include "manager/dolas_texture_manager.h"
#include "dolas_log_system_manager.h"
namespace Dolas
{
    namespace
    {
        uint32_t AlignTo(uint32_t value, uint32_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        bool CreateD3D12UploadBuffer(ID3D12Device* device, uint32_t size, const void* initial_data, ID3D12Resource** resource)
        {
            if (!device || !resource || size == 0)
            {
                return false;
            }

            D3D12_HEAP_PROPERTIES heap_properties = {};
            heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_properties.CreationNodeMask = 1;
            heap_properties.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC resource_desc = {};
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resource_desc.Width = size;
            resource_desc.Height = 1;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.MipLevels = 1;
            resource_desc.Format = DXGI_FORMAT_UNKNOWN;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = device->CreateCommittedResource(
                &heap_properties,
                D3D12_HEAP_FLAG_NONE,
                &resource_desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(resource));
            if (FAILED(hr))
            {
                LOG_ERROR("ShaderContext: failed to create D3D12 upload buffer, HRESULT: 0x{0:X}", hr);
                return false;
            }

            if (initial_data)
            {
                D3D12_RANGE read_range = { 0, 0 };
                void* mapped_data = nullptr;
                hr = (*resource)->Map(0, &read_range, &mapped_data);
                if (FAILED(hr))
                {
                    LOG_ERROR("ShaderContext: failed to map D3D12 upload buffer, HRESULT: 0x{0:X}", hr);
                    (*resource)->Release();
                    *resource = nullptr;
                    return false;
                }

                memcpy(mapped_data, initial_data, size);
                D3D12_RANGE written_range = { 0, size };
                (*resource)->Unmap(0, &written_range);
            }

            return true;
        }

        bool UpdateD3D12UploadBuffer(ID3D12Resource* resource, const void* data, uint32_t size, uint32_t offset)
        {
            if (!resource || !data || size == 0)
            {
                return false;
            }

            D3D12_RANGE read_range = { 0, 0 };
            void* mapped_data = nullptr;
            HRESULT hr = resource->Map(0, &read_range, &mapped_data);
            if (FAILED(hr))
            {
                LOG_ERROR("ShaderContext: failed to update D3D12 upload buffer, HRESULT: 0x{0:X}", hr);
                return false;
            }

            memcpy(static_cast<uint8_t*>(mapped_data) + offset, data, size);
            D3D12_RANGE written_range = { offset, offset + size };
            resource->Unmap(0, &written_range);
            return true;
        }
    }

    // Custom include handler to search files under PathUtils::GetShadersSourceDir()
    class DolasShaderInclude : public ID3DInclude
    {
    public:
        DolasShaderInclude()
        {
            m_root_dir = PathUtils::GetShadersSourceDir();
            if (!m_root_dir.empty())
            {
                // Ensure trailing slash
                char last = m_root_dir.back();
                if (last != '/' && last != '\\')
                {
                    m_root_dir.push_back('/');
                }
            }
        }

        virtual HRESULT STDMETHODCALLTYPE Open(
            D3D_INCLUDE_TYPE /*IncludeType*/,
            LPCSTR pFileName,
            LPCVOID /*pParentData*/,
            LPCVOID* ppData,
            UINT* pBytes) override
        {
            if (!pFileName || !ppData || !pBytes)
            {
                return E_INVALIDARG;
            }

            std::string full_path = m_root_dir + pFileName;

            std::ifstream file(full_path, std::ios::binary | std::ios::ate);
            if (!file.is_open())
            {
                // Failed to open include file
                return E_FAIL;
            }

            std::streamsize size = file.tellg();
            if (size <= 0)
            {
                return E_FAIL;
            }
            file.seekg(0, std::ios::beg);

            char* buffer = new char[static_cast<size_t>(size)];
            if (!file.read(buffer, size))
            {
                delete[] buffer;
                return E_FAIL;
            }

            *ppData = buffer;
            *pBytes = static_cast<UINT>(size);
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) override
        {
            const char* buffer = static_cast<const char*>(pData);
            delete[] buffer;
            return S_OK;
        }

    private:
        std::string m_root_dir;
    };
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
        if (m_global_constant_buffer)
        {
            m_global_constant_buffer->Release();
            m_global_constant_buffer = nullptr;
        }
        if (m_d3d12_global_constant_buffer)
        {
            m_d3d12_global_constant_buffer->Release();
            m_d3d12_global_constant_buffer = nullptr;
        }
        m_slot_to_d3d12_srv_map.clear();
        m_slot_to_d3d12_srv_cpu_map.clear();
	}

	ShaderBytecodeView ShaderContext::GetShaderBytecode() const
	{
		if (!m_d3d_shader_blob)
		{
			return {};
		}

		return { m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize() };
	}

	void ShaderContext::AnalyzeConstantBuffers(UInt constant_buffers_count)
	{
		for (UInt cb_index = 0; cb_index < constant_buffers_count; ++cb_index)
		{
			// ͨ��������ȡ��������������ӿ�
			ID3D11ShaderReflectionConstantBuffer* pConstantBuffer =
				m_d3d_shader_reflection->GetConstantBufferByIndex(static_cast<UINT>(cb_index));

			if (pConstantBuffer == nullptr)
			{
				LOG_ERROR("�����޷���ȡ���������� {0} �ķ���ӿ�\n", cb_index);
				continue;
			}

			// ������������������
			ConstantBufferInfo cb_info;

			D3D11_SHADER_BUFFER_DESC cb_desc;
			pConstantBuffer->GetDesc(&cb_desc);
			cb_info.name = cb_desc.Name ? cb_desc.Name : "";
			cb_info.size = cb_desc.Size;
			cb_info.variable_count = cb_desc.Variables;
			
			for (UInt variable_index = 0; variable_index < cb_desc.Variables; ++variable_index)
			{
				ID3D11ShaderReflectionVariable* pVariable =
					pConstantBuffer->GetVariableByIndex(static_cast<UINT>(variable_index));
				if (pVariable == nullptr)
				{
					LOG_ERROR("�����޷���ȡ���������� {0} �б��� {1} �ķ���ӿ�\n", cb_index, variable_index);
					continue;
				}
				D3D11_SHADER_VARIABLE_DESC var_desc;
				pVariable->GetDesc(&var_desc);
				ConstantBufferVariableInfo var_info;
				var_info.name = var_desc.Name ? var_desc.Name : "";
				var_info.start_offset = var_desc.StartOffset;
				var_info.size = var_desc.Size;
				var_info.flags = var_desc.uFlags;
				cb_info.variable_descs.push_back(std::move(var_info));
			}

			m_shader_reflection_info.constant_buffer_descs.push_back(cb_info);
		}
	}

	void ShaderContext::GenerateReflectionAndDesc()
	{
		HR(D3DReflect(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&m_d3d_shader_reflection));
		
		D3D11_SHADER_DESC shader_desc = {};
		HR(m_d3d_shader_reflection->GetDesc(&shader_desc));
		m_shader_reflection_info.constant_buffer_count = shader_desc.ConstantBuffers;
		m_shader_reflection_info.bound_resource_count = shader_desc.BoundResources;
		m_shader_reflection_info.instruction_count = shader_desc.InstructionCount;

		AnalyzeConstantBuffers(m_shader_reflection_info.constant_buffer_count);

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
        if (m_d3d12_global_constant_buffer)
        {
            m_d3d12_global_constant_buffer->Release();
            m_d3d12_global_constant_buffer = nullptr;
        }

		// 在反射出来的 CB 里查找名为 "GlobalConstants" 的 cbuffer
		const char* kGlobalCBName = "GlobalConstants";
		const ConstantBufferInfo* target_cb_info = nullptr;

		for (const auto& cb_info : m_shader_reflection_info.constant_buffer_descs)
		{
			if (cb_info.name == kGlobalCBName)
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

        // 初始化 CPU 端缓存区，用来存放 GlobalConstants 的原始字节数据
        m_global_cb_data.clear();
        m_global_cb_data.resize(target_cb_info->size);

		ID3D11Device* device = g_dolas_engine.m_rhi->GetD3D11Device();
        if (device)
        {
            // Legacy D3D11 mirror. The active renderer consumes m_d3d12_global_constant_buffer.
            D3D11_BUFFER_DESC cbd = {};
            cbd.ByteWidth = target_cb_info->size; // HLSL 编译器保证是 16 字节对齐
            cbd.Usage = D3D11_USAGE_DYNAMIC;
            cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbd.MiscFlags = 0;

            HR(device->CreateBuffer(&cbd, nullptr, &m_global_constant_buffer));
        }

        RenderHardwareInterface* render_hardware_interface = g_dolas_engine.m_render_hardware_interface;
        ID3D12Device* d3d12_device = render_hardware_interface ? render_hardware_interface->GetDevice() : nullptr;
        if (d3d12_device)
        {
            const uint32_t d3d12_cb_size = AlignTo(target_cb_info->size, 256);
            std::vector<uint8_t> d3d12_initial_data(d3d12_cb_size, 0);
            if (!m_global_cb_data.empty())
            {
                memcpy(d3d12_initial_data.data(), m_global_cb_data.data(), m_global_cb_data.size());
            }

            if (!CreateD3D12UploadBuffer(d3d12_device, d3d12_cb_size, d3d12_initial_data.data(), &m_d3d12_global_constant_buffer))
            {
                LOG_ERROR("Failed to create D3D12 GlobalConstants CB for shader {0}", m_file_path);
            }
        }

		LOG_INFO("Created GlobalConstants CB for shader {0}, size = {1} bytes", m_file_path, target_cb_info->size);
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

		// 材质阶段通常只关心 TextureID，这里也顺便填充 SRV 映射，方便后续直接绑定
		m_slot_to_texture_map[slot] = texture_id;
        if (ID3D11ShaderResourceView* srv = texture->GetShaderResourceView())
        {
            m_slot_to_srv_map[slot] = srv;
        }
        if (texture->HasD3D12Srv())
        {
            m_slot_to_d3d12_srv_cpu_map[slot] = texture->GetD3D12SrvCpuHandle();
            m_slot_to_d3d12_srv_map[slot] = texture->GetD3D12SrvGpuHandle();
        }
	}

	void ShaderContext::SetShaderResourceView(size_t slot, Texture* texture)
	{
		DOLAS_RETURN_IF_NULL(texture);

        if (ID3D11ShaderResourceView* srv = texture->GetShaderResourceView())
        {
            m_slot_to_srv_map[slot] = srv;
        }
        if (texture->HasD3D12Srv())
        {
            m_slot_to_d3d12_srv_cpu_map[slot] = texture->GetD3D12SrvCpuHandle();
            m_slot_to_d3d12_srv_map[slot] = texture->GetD3D12SrvGpuHandle();
        }
	}

	void ShaderContext::dumpShaderReflectionInfo() const
	{
		LOG_INFO("file = {0}, entry = {1}", m_file_path, m_entry_point);
		LOG_INFO("  ConstantBuffers = {0}, BoundResources = {1}, Instructions = {2}",
			m_shader_reflection_info.constant_buffer_count,
			m_shader_reflection_info.bound_resource_count,
			m_shader_reflection_info.instruction_count);

		for (size_t cb_index = 0; cb_index < m_shader_reflection_info.constant_buffer_descs.size(); ++cb_index)
		{
			const auto& cb_info = m_shader_reflection_info.constant_buffer_descs[cb_index];
			LOG_INFO("name = {0}, size = {1} bytes, variables = {2}",
				cb_info.name,
				cb_info.size,
				cb_info.variable_count);

			for (size_t var_index = 0; var_index < cb_info.variable_descs.size(); ++var_index)
			{
				const auto& var_desc = cb_info.variable_descs[var_index];

				LOG_INFO("    Var[{0}]: name = {1}, startOffset = {2}, size = {3} bytes, flags = {4}",
					var_index,
					var_desc.name,
					var_desc.start_offset,
					var_desc.size,
					var_desc.flags);
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

            if (ID3D11ShaderResourceView* srv = tex->GetShaderResourceView())
            {
                m_slot_to_srv_map[slot] = srv;
            }
            if (tex->HasD3D12Srv())
            {
                m_slot_to_d3d12_srv_cpu_map[slot] = tex->GetD3D12SrvCpuHandle();
                m_slot_to_d3d12_srv_map[slot] = tex->GetD3D12SrvGpuHandle();
            }
        }
	}

    void ShaderContext::SetGlobalVariable(const std::string& name, const Vector4& values)
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
            if (cb_info.name == kGlobalCBName)
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
            if (name != var_desc.name)
            {
                continue;
            }

            // 计算要拷贝的字节数（不超过变量大小）
            size_t src_bytes = sizeof(Vector4);
            size_t copy_bytes = std::min<size_t>(src_bytes, var_desc.size);

            if (copy_bytes == 0)
            {
                return;
            }

            // 写入到 CPU 端 Global CB 缓冲区中相应的偏移位置
            uint8_t* dst = m_global_cb_data.data() + var_desc.start_offset;
            std::memcpy(dst, &values, copy_bytes);
            if (m_d3d12_global_constant_buffer)
            {
                UpdateD3D12UploadBuffer(
                    m_d3d12_global_constant_buffer,
                    &values,
                    static_cast<uint32_t>(copy_bytes),
                    var_desc.start_offset);
            }
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
        DolasShaderInclude include_handler;
		HR(D3DCompileFromFile(
			StringUtil::StringToWString(file_path).c_str(), // file path
			nullptr, // macros
			&include_handler, // include
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
        if (device)
        {
            HR(device->CreateVertexShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_vertex_shader));
        }

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
        DolasShaderInclude include_handler;
		HR(D3DCompileFromFile(
			StringUtil::StringToWString(file_path).c_str(), // file path
			nullptr, // macros
			&include_handler, // include
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
        if (device)
        {
            HR(device->CreatePixelShader(m_d3d_shader_blob->GetBufferPointer(), m_d3d_shader_blob->GetBufferSize(), nullptr, &m_d3d_pixel_shader));
        }

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

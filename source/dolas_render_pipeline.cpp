#include <string>
#include <iostream>
#include <d3dcompiler.h>
#include <cstddef>  // for offsetof

#include "base/dolas_paths.h"
#include "base/dolas_string_utils.h"
#include "dolas_render_pipeline.h"
#include "DDSTextureLoader11.h"
namespace Dolas
{
    RenderPipeline::RenderPipeline()
    {

    }

    RenderPipeline::~RenderPipeline()
    {

    }

    bool RenderPipeline::Initialize()
    {
        return true;
    }

    void RenderPipeline::Render(DolasRHI* rhi)
    {
        ClearPass(rhi);
        GBufferPass(rhi);
        DeferredShadingPass(rhi);
        ForwardShadingPass(rhi);
        PostProcessPass(rhi);
        PresentPass(rhi);
    }

    void RenderPipeline::ClearPass(DolasRHI* rhi)
    {
        const FLOAT clear_color[4] = {0.4f, 0.6f, 0.8f, 1.0f};
        rhi->m_d3d_immediate_context->ClearRenderTargetView(rhi->m_render_target_view, clear_color);
    }

    void RenderPipeline::GBufferPass(DolasRHI* rhi)
    {
		
    }

    void RenderPipeline::DeferredShadingPass(DolasRHI* rhi)
    {

    }

	void RenderPipeline::ForwardShadingPass(DolasRHI* rhi)
	{
        // 获取shader
        std::string vs_shader_file_path = Dolas::Paths::GetBinDir() + "/shaders/test_VS_Entry_Test.cso";
        std::string ps_shader_file_path = Dolas::Paths::GetBinDir() + "/shaders/test_PS_Entry_Test.cso";
        std::wstring wsting_vs_shader_file_path = StringUtils::StringToWString(vs_shader_file_path);
        std::wstring wsting_ps_shader_file_path = StringUtils::StringToWString(ps_shader_file_path);

        ID3DBlob* vs_blob = nullptr;
        ID3DBlob* ps_blob = nullptr;

        HRESULT hr = D3DReadFileToBlob(wsting_vs_shader_file_path.c_str(), &vs_blob);
		if (FAILED(hr))
		{
			std::cerr << "Failed to load vertex shader: " << vs_shader_file_path << std::endl;
            return;
		}

        hr = D3DReadFileToBlob(wsting_ps_shader_file_path.c_str(), &ps_blob);
		if (FAILED(hr))
		{
			std::cerr << "Failed to load pixel shader: " << ps_shader_file_path << std::endl;
            vs_blob->Release();
            return;
		}

        // 创建顶点着色器和像素着色器
        ID3D11VertexShader* vertex_shader = nullptr;
        ID3D11PixelShader* pixel_shader = nullptr;

        hr = rhi->m_d3d_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertex_shader);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create vertex shader" << std::endl;
            vs_blob->Release();
            ps_blob->Release();
            return;
        }

        hr = rhi->m_d3d_device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &pixel_shader);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create pixel shader" << std::endl;
            vertex_shader->Release();
            vs_blob->Release();
            ps_blob->Release();
            return;
        }
        
        // 准备正方形顶点缓冲区
        // 定义顶点结构
        struct Vertex
        {
            float position[3];  // x, y, z
            float texcoord[2];  // u, v
        };

        // 正方形顶点数据（两个三角形组成）
        Vertex quad_vertices[] = {
            // 第一个三角形
            { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f} },  // 左上
            { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} },  // 右上
            { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} },  // 左下
            
            // 第二个三角形
            { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f} },  // 右上
            { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} },  // 右下
            { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} }   // 左下
        };

        // 创建顶点缓冲区
        D3D11_BUFFER_DESC vertex_buffer_desc = {};
        vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        vertex_buffer_desc.ByteWidth = sizeof(quad_vertices);
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA vertex_init_data = {};
        vertex_init_data.pSysMem = quad_vertices;

        ID3D11Buffer* vertex_buffer = nullptr;
        hr = rhi->m_d3d_device->CreateBuffer(&vertex_buffer_desc, &vertex_init_data, &vertex_buffer);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create vertex buffer" << std::endl;
            vertex_shader->Release();
            pixel_shader->Release();
            vs_blob->Release();
            ps_blob->Release();
            return;
        }

        // 创建输入布局 - 语义名称必须与着色器完全匹配
        D3D11_INPUT_ELEMENT_DESC input_layout_desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        // 调试信息：打印顶点结构大小
        std::cout << "Vertex size: " << sizeof(Vertex) << " bytes" << std::endl;
        std::cout << "Position offset: 0, size: " << sizeof(float) * 3 << " bytes" << std::endl;
        std::cout << "Texcoord offset: " << offsetof(Vertex, texcoord) << ", size: " << sizeof(float) * 2 << " bytes" << std::endl;

        ID3D11InputLayout* input_layout = nullptr;
        hr = rhi->m_d3d_device->CreateInputLayout(
            input_layout_desc,
            ARRAYSIZE(input_layout_desc),
            vs_blob->GetBufferPointer(),
            vs_blob->GetBufferSize(),
            &input_layout
        );

        if (FAILED(hr))
        {
            std::cerr << "Failed to create input layout. HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
            std::cerr << "Make sure vertex shader input layout matches the input element description" << std::endl;
            vertex_buffer->Release();
            vertex_shader->Release();
            pixel_shader->Release();
            vs_blob->Release();
            ps_blob->Release();
            return;
        }

        // 设置渲染目标
        rhi->m_d3d_immediate_context->OMSetRenderTargets(1, &rhi->m_render_target_view, nullptr);

        // 设置视口
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = 1920.0f;    // 匹配窗口宽度
        viewport.Height = 1080.0f;   // 匹配窗口高度
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        rhi->m_d3d_immediate_context->RSSetViewports(1, &viewport);

        // 设置读取纹理-dds格式
        ID3D11ShaderResourceView* albedo_texture_srv = nullptr;
        std::wstring albedo_path = Dolas::Paths::BuildContentPathW("textures/albedo.dds");
        hr = DirectX::CreateDDSTextureFromFile(rhi->m_d3d_device, albedo_path.c_str(), nullptr, &albedo_texture_srv);
        if (FAILED(hr))
        {
            std::cerr << "Failed to load texture: " << Dolas::StringUtils::WStringToString(albedo_path) << std::endl;
        }

        // 启动渲染
        // 设置输入布局
        rhi->m_d3d_immediate_context->IASetInputLayout(input_layout);

        // 设置顶点缓冲区
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        rhi->m_d3d_immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

        // 设置图元拓扑
        rhi->m_d3d_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 设置着色器
        rhi->m_d3d_immediate_context->VSSetShader(vertex_shader, nullptr, 0);
        rhi->m_d3d_immediate_context->PSSetShader(pixel_shader, nullptr, 0);

        // 创建采样器状态
        D3D11_SAMPLER_DESC sampler_desc = {};
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.MinLOD = 0;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

        ID3D11SamplerState* sampler_state = nullptr;
        hr = rhi->m_d3d_device->CreateSamplerState(&sampler_desc, &sampler_state);
        if (FAILED(hr))
        {
            std::cerr << "Failed to create sampler state" << std::endl;
        }

        // 设置纹理和采样器
        if (albedo_texture_srv)
        {
            rhi->m_d3d_immediate_context->PSSetShaderResources(0, 1, &albedo_texture_srv);
        }
        if (sampler_state)
        {
            rhi->m_d3d_immediate_context->PSSetSamplers(0, 1, &sampler_state);
        }

        // 绘制正方形（6个顶点，2个三角形）
        rhi->m_d3d_immediate_context->Draw(6, 0);

        // 清理资源
        if (albedo_texture_srv) albedo_texture_srv->Release();
        if (sampler_state) sampler_state->Release();
        input_layout->Release();
        vertex_buffer->Release();
        vertex_shader->Release();
        pixel_shader->Release();
        vs_blob->Release();
        ps_blob->Release();
	}
    
    void RenderPipeline::PostProcessPass(DolasRHI* rhi)
    {

    }

    void RenderPipeline::PresentPass(DolasRHI* rhi)
    {
        rhi->m_swap_chain->Present(0, 0);
    }
} // namespace Dolas

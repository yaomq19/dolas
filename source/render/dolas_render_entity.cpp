#include <fstream>
#include <iostream>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include "base/dolas_paths.h"
#include "base/dolas_base.h"
#include "core/dolas_engine.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_mesh.h"
#include "render/dolas_material.h"
#include "base/dolas_dx_trace.h"
#include "render/dolas_shader.h"

namespace Dolas
{
    RenderEntity::RenderEntity()
    {

    }

    RenderEntity::~RenderEntity()
    {

    }

    bool RenderEntity::Clear()
    {
        return true;
    }

    void RenderEntity::Draw(DolasRHI* rhi)
    {
        if (m_mesh == nullptr || m_material == nullptr)
            return;

        // 根据 mesh 创建顶点缓冲区
        // 设置顶点缓冲区描述
            // hack: 默认的 3+2+3 构造（顶点、uv、法线）

        D3D11_BUFFER_DESC vbd;
        ZeroMemory(&vbd, sizeof(vbd));
        vbd.Usage = D3D11_USAGE_IMMUTABLE;
        vbd.ByteWidth = m_mesh->m_final_vertices.size() * sizeof(float);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;
        // 新建顶点缓冲区
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = m_mesh->m_final_vertices.data();

            ID3D11Buffer* vertex_buffer = nullptr;
        HR(rhi->m_d3d_device->CreateBuffer(&vbd, &InitData, &vertex_buffer));

            // 根据 mesh 创建索引缓冲区
        D3D11_BUFFER_DESC ibd;
        ZeroMemory(&ibd, sizeof(ibd));
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = m_mesh->m_indices.size() * sizeof(unsigned int);
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;  // 修正绑定标志
        ibd.CPUAccessFlags = 0;

            InitData.pSysMem = m_mesh->m_indices.data();

        ID3D11Buffer* index_buffer = nullptr;
        HR(rhi->m_d3d_device->CreateBuffer(&ibd, &InitData, &index_buffer));  // 修正缓冲区描述

        // 绑定 VB 和 IB
        UINT stride = (3 + 2 + 3) * sizeof(float);	// 跨越字节数
        UINT offset = 0;						// 起始偏移量
        rhi->m_d3d_immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        rhi->m_d3d_immediate_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);

        // 设置 IA
        rhi->m_d3d_immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11InputLayout* input_layout = nullptr;

        D3D11_INPUT_ELEMENT_DESC local_layout[] =
        {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };


        HR(
            rhi->m_d3d_device->CreateInputLayout(
                local_layout,
                3,
                m_material->GetVertexShader()->GetD3DShaderBlob()->GetBufferPointer(),
                m_material->GetVertexShader()->GetD3DShaderBlob()->GetBufferSize(),
                &input_layout)
        );

        rhi->m_d3d_immediate_context->IASetInputLayout(input_layout);
        // 绑定 Shader
        rhi->m_d3d_immediate_context->VSSetShader(m_material->GetVertexShader()->GetD3DVertexShader(), nullptr, 0);
        rhi->m_d3d_immediate_context->PSSetShader(m_material->GetPixelShader()->GetD3DPixelShader(), nullptr, 0);
        // 设置 RTV 和 DSV
        rhi->m_d3d_immediate_context->OMSetRenderTargets(1, &rhi->m_render_target_view, nullptr);
        // 设置渲染状态
        // 
        // 发起DC
        rhi->m_d3d_immediate_context->DrawIndexed(m_mesh->m_indices.size(), 0, 0);
    }
}
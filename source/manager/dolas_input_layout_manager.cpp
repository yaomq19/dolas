#include "manager/dolas_input_layout_manager.h"

#include <vector>

#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_shader_manager.h"
#include "render/dolas_shader.h"
#include "base/dolas_base.h"

namespace Dolas
{
    InputLayoutManager::InputLayoutManager()
    {
    }

    InputLayoutManager::~InputLayoutManager()
    {
        Clear();
    }

    bool InputLayoutManager::Initialize()
    {
        return CreateLayouts();
    }

    bool InputLayoutManager::Clear()
    {
        for (auto& it : m_input_layouts)
        {
            if (it.second)
            {
                it.second->Release();
            }
        }
        m_input_layouts.clear();
        return true;
    }

    ID3D11InputLayout* InputLayoutManager::GetInputLayoutByType(const DolasInputLayoutType& type) const
    {
        auto it = m_input_layouts.find(type);
        if (it == m_input_layouts.end()) return nullptr;
        return it->second;
    }

    bool InputLayoutManager::CreateLayouts()
    {
        DOLAS_RETURN_FALSE_IF_NULL(g_dolas_engine.m_rhi);
        DOLAS_RETURN_FALSE_IF_NULL(g_dolas_engine.m_shader_manager);

        // 为 POS_3_UV_2_NORM_3 使用 solid.vs.hlsl VS 反射创建布局
        {
            VertexShader* vs = g_dolas_engine.m_shader_manager->GetOrCreateVertexShader("solid.vs.hlsl", "VS");
            DOLAS_RETURN_FALSE_IF_NULL(vs);
            ID3DBlob* vs_blob = vs->GetD3DShaderBlob();
            DOLAS_RETURN_FALSE_IF_NULL(vs_blob);

            std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
            descs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 });
            descs.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 });
            descs.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20,  D3D11_INPUT_PER_VERTEX_DATA, 0 });

            ID3D11InputLayout* layout = nullptr;
            HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateInputLayout(
                descs.data(), static_cast<UINT>(descs.size()),
                vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                &layout);
            if (FAILED(hr)) return false;

            m_input_layouts[DolasInputLayoutType::POS_3_UV_2_NORM_3] = layout;
        }

        // 为 POS_3_UV_2 使用 deferred_shading.vs.hlsl VS 反射创建布局
        {
            VertexShader* vs = g_dolas_engine.m_shader_manager->GetOrCreateVertexShader("deferred_shading.vs.hlsl", "VS");
            DOLAS_RETURN_FALSE_IF_NULL(vs);
            ID3DBlob* vs_blob = vs->GetD3DShaderBlob();
            DOLAS_RETURN_FALSE_IF_NULL(vs_blob);

            std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
            descs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 });
            descs.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 });

            ID3D11InputLayout* layout = nullptr;
            HRESULT hr = g_dolas_engine.m_rhi->m_d3d_device->CreateInputLayout(
                descs.data(), static_cast<UINT>(descs.size()),
                vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                &layout);
            if (FAILED(hr)) return false;

            m_input_layouts[DolasInputLayoutType::POS_3_UV_2] = layout;
        }

        return true;
    }
}



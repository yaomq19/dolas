#include "function/FrameWork/Dolas.h"
#include "function/Render/TextureManager/TextureManager.h"

namespace Dolas
{
    TextureManager::TextureManager()
    {
    }

    TextureManager::~TextureManager()
    {
    }

    bool TextureManager::Initialize()
    {
        // 获取 swapchain 的后台buffer
        g_dolas_context.m_d3d_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_back_buffer);
        return true;
    }
}
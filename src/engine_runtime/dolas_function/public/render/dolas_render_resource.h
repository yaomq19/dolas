#ifndef DOLAS_RENDER_RESOURCE_H
#define DOLAS_RENDER_RESOURCE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "dolas_hash.h"

namespace Dolas
{
    class Texture;
    class Buffer;

    class RenderResource
    {
        friend class RenderResourceManager;
    public:
        TextureID m_gbuffer_a_id = TEXTURE_ID_EMPTY;
        TextureID m_gbuffer_b_id = TEXTURE_ID_EMPTY;
        TextureID m_gbuffer_c_id = TEXTURE_ID_EMPTY;
        TextureID m_gbuffer_d_id = TEXTURE_ID_EMPTY;
        TextureID m_depth_stencil_id = TEXTURE_ID_EMPTY;
        TextureID m_scene_result_id = TEXTURE_ID_EMPTY;
    }; // class RenderResource
} // namespace Dolas

#endif // DOLAS_RENDER_RESOURCE_H

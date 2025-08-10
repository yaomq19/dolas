#ifndef DOLAS_RENDER_RESOURCE_MANAGER_H
#define DOLAS_RENDER_RESOURCE_MANAGER_H

#include "common/dolas_hash.h"
#include "render/dolas_render_resource.h"

namespace Dolas
{
    class RenderResourceManager
    {
    public:
        RenderResourceManager();
        ~RenderResourceManager();
        Bool Initialize();
        Bool Clear();
        RenderResource* GetRenderResource(RenderResourceID render_resource_id);
        Bool CreateRenderResource(RenderResourceID render_resource_id);
    protected:
        std::unordered_map<RenderResourceID, RenderResource*> m_render_resources;
    };
}

#endif
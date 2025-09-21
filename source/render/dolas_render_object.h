#ifndef DOLAS_RENDER_OBJECT_H
#define DOLAS_RENDER_OBJECT_H

#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "common/dolas_hash.h"
#include "core/dolas_rhi.h"
#include "core/dolas_math.h"
#include "render/dolas_transform.h"
namespace Dolas
{
    /**
     * @brief RenderObject represents a single renderable object in the scene
     * 
     * RenderObject is a higher-level abstraction that contains:
     * - Transform information (position, rotation, scale)
     * - Reference to a RenderEntity (mesh + material)
     * - Visibility and culling information
     * - Per-object constant buffer data
     */
    class RenderObject
    {
        friend class RenderObjectManager;
    public:
        RenderObject();
        ~RenderObject();

        void Draw(class DolasRHI* rhi);
        void SetRenderModelID(RenderModelID model_id);
        RenderModelID GetRenderModelID() const;
    protected:
        RenderObjectID m_object_id = RENDER_OBJECT_ID_EMPTY;
        RenderModelID m_render_model_id = RENDER_MODEL_ID_EMPTY;
        
        // Transform data
        Transform m_transform;
    };
    
} // namespace Dolas

#endif // DOLAS_RENDER_OBJECT_H

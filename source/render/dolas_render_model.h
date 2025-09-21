#ifndef DOLAS_RENDER_MODEL_H
#define DOLAS_RENDER_MODEL_H

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
    class RenderObject;
    
    /**
     * @brief RenderModel represents a collection of RenderObjects forming a complex model
     * 
     * RenderModel is used for:
     * - Multi-part models (e.g., a car with wheels, doors, etc.)
     * - Hierarchical objects with parent-child relationships
     * - LOD (Level of Detail) management
     * - Animation and skeletal systems
     * - Instanced rendering of similar objects
     */
    class RenderModel
    {
        friend class RenderModelManager;
        
    public:
        RenderModel();
        ~RenderModel();
        
        void Draw(class DolasRHI* rhi, Transform transform);
        
    protected:
        RenderModelID m_model_id = RENDER_MODEL_ID_EMPTY;
        RenderEntityID m_render_entity_id = RENDER_ENTITY_ID_EMPTY;
    };
    
} // namespace Dolas

#endif // DOLAS_RENDER_MODEL_H

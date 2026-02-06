#ifndef DOLA_RENDER_ENTITY_H
#define DOLA_RENDER_ENTITY_H

#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "dolas_hash.h"
#include "render/dolas_rhi.h"
#include "render/dolas_transform.h"

namespace Dolas
{
    class Material;
    struct RenderComponent
    {
        RenderPrimitiveID m_render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
        MaterialID m_material_id = MATERIAL_ID_EMPTY;
    };

    class RenderEntity
    {
        friend class RenderEntityManager;
    public:
        RenderEntity();
        ~RenderEntity();
        bool Clear();
        void Draw(DolasRHI* rhi);

        void AddComponent(RenderPrimitiveID mesh_id, MaterialID material_id);
    protected:
        RenderEntityID m_file_id = RENDER_ENTITY_ID_EMPTY;
        std::vector<RenderComponent> m_components;
		Pose m_pose = Pose();
    };
} // namespace Dolas

#endif // DOLA_RENDER_ENTITY_H
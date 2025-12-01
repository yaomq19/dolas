#ifndef DOLA_RENDER_ENTITY_H
#define DOLA_RENDER_ENTITY_H

#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "common/dolas_hash.h"
#include "core/dolas_rhi.h"
#include "render/dolas_transform.h"

namespace Dolas
{
    class Material;
    class RenderEntity
    {
        friend class RenderEntityManager;
    public:
        RenderEntity();
        ~RenderEntity();
        bool Clear();
        void Draw(DolasRHI* rhi);

        void SetMeshID(RenderPrimitiveID mesh_id);
        void SetMaterialID(MaterialID material_id);
    protected:
        RenderEntityID m_file_id = RENDER_ENTITY_ID_EMPTY;
        RenderPrimitiveID m_render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
        MaterialID m_material_id = MATERIAL_ID_EMPTY;
		Pose m_pose = Pose();
    };
} // namespace Dolas

#endif // DOLA_RENDER_ENTITY_H
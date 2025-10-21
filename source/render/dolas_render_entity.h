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
    class Mesh;
    class Material;
    class RenderEntity
    {
        friend class RenderEntityManager;
    public:
        RenderEntity();
        ~RenderEntity();
        bool Clear();
        void Draw(DolasRHI* rhi);

        void SetMeshID(MeshID mesh_id);
        void SetMaterialID(MaterialID material_id);
    protected:
        RenderEntityID m_file_id = RENDER_ENTITY_ID_EMPTY;
        MeshID m_mesh_id = MESH_ID_EMPTY;
        MaterialID m_material_id = MATERIAL_ID_EMPTY;
    };
} // namespace Dolas

#endif // DOLA_RENDER_ENTITY_H
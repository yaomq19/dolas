#ifndef DOLA_RENDER_ENTITY_H
#define DOLA_RENDER_ENTITY_H

#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "core/dolas_rhi.h"

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
    private:
        std::string m_file_path;
        Mesh* m_mesh = nullptr;
        Material* m_material = nullptr;
    };
} // namespace Dolas

#endif // DOLA_RENDER_ENTITY_H
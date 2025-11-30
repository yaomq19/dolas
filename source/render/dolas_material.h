#ifndef DOLAS_MATERIAL_H
#define DOLAS_MATERIAL_H

#include <string>
#include <unordered_map>
#include <d3d11.h>
#include "common/dolas_hash.h"

namespace Dolas
{
    class Material
    {
        friend class MaterialManager;
    public:
        Material();
        ~Material();
        class VertexContext* GetVertexContext();
        class PixelContext* GetPixelContext();
    protected:
        MaterialID m_file_id;
        class VertexContext* m_vertex_context = nullptr;
        class PixelContext* m_pixel_context = nullptr;
    }; // class Material
} // namespace Dolas

#endif // DOLAS_MATERIAL_H
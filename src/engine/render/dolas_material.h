#ifndef DOLAS_MATERIAL_H
#define DOLAS_MATERIAL_H

#include <string>
#include <unordered_map>
#include <d3d11.h>
#include <memory>
#include "common/dolas_hash.h"
#include "render/dolas_shader.h"
namespace Dolas
{
    class Material
    {
        friend class MaterialManager;
    public:
        Material();
        ~Material();
        std::shared_ptr<VertexContext> GetVertexContext();
        std::shared_ptr<PixelContext> GetPixelContext();
    protected:
        MaterialID m_file_id;
        std::shared_ptr<VertexContext> m_vertex_context{ nullptr };
        std::shared_ptr<PixelContext> m_pixel_context{ nullptr };
    }; // class Material
} // namespace Dolas

#endif // DOLAS_MATERIAL_H
#ifndef DOLAS_MATERIAL_MANAGER_H
#define DOLAS_MATERIAL_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "base/dolas_base.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    class Material;
    class VertexContext;
    class PixelContext;

    struct GlobalMaterials
    {
        MaterialID m_deferred_shading = MATERIAL_ID_EMPTY;
        MaterialID m_sky_box_material_id = MATERIAL_ID_EMPTY;
        MaterialID m_debug_draw_material_id = MATERIAL_ID_EMPTY;
    };

    class MaterialManager
    {
    public:
        MaterialManager();
        ~MaterialManager();
        bool Initialize();
        bool Clear();
        void InitializeGlobalMaterial();
        MaterialID CreateMaterial(const std::string& file_name);
        Material* GetMaterialByID(MaterialID material_id);
        Material* GetDeferredShadingMaterial();
        Material* GetSkyBoxMaterial();
        Material* GetDebugDrawMaterial();
    private:
        std::shared_ptr<VertexContext> CreateVertexContext(const std::string& file_path, const std::string& entry_point);
        std::shared_ptr<PixelContext>  CreatePixelContext(const std::string& file_path, const std::string& entry_point);

        std::unordered_map<MaterialID, Material*> m_materials;
        GlobalMaterials m_global_materials;
    };// class MaterialManager
}// namespace Dolas
#endif // DOLAS_MATERIAL_MANAGER_H
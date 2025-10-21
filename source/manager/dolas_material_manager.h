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
    struct GlobalMaterials
    {
        MaterialID m_deferred_shading = MATERIAL_ID_EMPTY;
        MaterialID m_sky_box_material_id = MATERIAL_ID_EMPTY;
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
        Material* GetMaterial(MaterialID material_id);
        MaterialID GetDeferredShadingMaterialID();
        MaterialID GetSkyBoxMaterialID();
    private:
        std::unordered_map<MaterialID, Material*> m_materials;

        GlobalMaterials m_global_materials;
    };// class MaterialManager
}// namespace Dolas
#endif // DOLAS_MATERIAL_MANAGER_H
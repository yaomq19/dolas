#ifndef DOLAS_MATERIAL_MANAGER_H
#define DOLAS_MATERIAL_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <array>
#include "dolas_base.h"
#include "dolas_hash.h"
namespace Dolas
{
    class AssetPath;
    class Material;
    class VertexContext;
    class PixelContext;

    enum class GlobalMaterialType : UInt
    {
        DeferredShading,
        SkyBox,
        DebugDraw,
        Count
    };

    class MaterialManager
    {
    public:
        MaterialManager();
        ~MaterialManager();
        bool Initialize();
        bool Clear();
        MaterialID CreateMaterial(const AssetPath& asset_path);
        Material* GetMaterialByID(MaterialID material_id);
        Material* GetGlobalMaterial(GlobalMaterialType global_material_type);
    private:
        bool InitializeGlobalMaterials();
        std::shared_ptr<VertexContext> CreateVertexContext(const AssetPath& asset_path, const std::string& entry_point);
        std::shared_ptr<PixelContext>  CreatePixelContext(const AssetPath& asset_path, const std::string& entry_point);

        std::unordered_map<MaterialID, Material*> m_materials;
        std::array<MaterialID, static_cast<UInt>(GlobalMaterialType::Count)> m_global_materials{};
    };// class MaterialManager
}// namespace Dolas
#endif // DOLAS_MATERIAL_MANAGER_H

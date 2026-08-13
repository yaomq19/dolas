#ifndef DOLAS_ENTITY_ASSET_H
#define DOLAS_ENTITY_ASSET_H

#include <cstdint>
#include <string_view>
#include <vector>

#include "asset_types/mesh_asset.h"
#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"

namespace Dolas
{
    struct EntityAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.entity"};
        static constexpr std::string_view kFileSuffix{".entity"};
        static constexpr std::uint32_t kSchemaVersion{1};

        std::vector<AssetRef<MeshAssetDesc>> meshes;
    };

    template<>
    struct AssetReflection<EntityAssetDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = EntityAssetDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::meshes>("meshes"));
        }
    };
}

#endif // DOLAS_ENTITY_ASSET_H

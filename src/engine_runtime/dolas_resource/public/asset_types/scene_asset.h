#ifndef DOLAS_SCENE_ASSET_H
#define DOLAS_SCENE_ASSET_H

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "asset_types/entity_asset.h"
#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"
#include "dolas_math.h"

namespace Dolas
{
    struct SceneEntityDesc
    {
        std::optional<AssetRef<EntityAssetDesc>> entity;
        Vector3 position{};
        Vector4 rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};
    };

    template<>
    struct AssetReflection<SceneEntityDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = SceneEntityDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::entity>(
                    "entities",
                    AssetFieldOptions{.required = true, .display_name = "Entity"}),
                MakeAssetField<2, &T::position>("entity_positions"),
                MakeAssetField<3, &T::rotation>("entity_rotations"),
                MakeAssetField<4, &T::scale>("entity_scales"));
        }
    };

    struct SceneAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.scene"};
        static constexpr std::string_view kFileSuffix{".scene"};
        static constexpr std::uint32_t kSchemaVersion{1};

        std::vector<SceneEntityDesc> entities;
    };

    template<>
    struct AssetReflection<SceneAssetDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = SceneAssetDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::entities>("entities"));
        }
    };
}

#endif // DOLAS_SCENE_ASSET_H

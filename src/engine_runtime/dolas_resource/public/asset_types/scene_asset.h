#ifndef DOLAS_SCENE_ASSET_H
#define DOLAS_SCENE_ASSET_H

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "asset_types/entity_asset.h"
#include "dolas_asset_ref.h"
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

    struct SceneAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.scene"};
        static constexpr std::string_view kFileSuffix{".scene"};
        static constexpr std::uint32_t kSchemaVersion{1};

        std::vector<SceneEntityDesc> entities;
    };
}

#endif // DOLAS_SCENE_ASSET_H

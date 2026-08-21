#ifndef DOLAS_MESH_ASSET_H
#define DOLAS_MESH_ASSET_H

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "asset_types/material_asset.h"
#include "dolas_asset_ref.h"
#include "dolas_base.h"

namespace Dolas
{
    enum class TopologyType : UInt
    {
        TriangleList = 0,
        TriangleStrip = 1,
    };

    struct MeshAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.mesh"};
        static constexpr std::string_view kFileSuffix{".mesh"};
        static constexpr std::uint32_t kSchemaVersion{1};

        std::vector<Float> position;
        std::vector<Float> normal;
        std::vector<Float> tangent;
        std::vector<Float> uv0;
        std::vector<Float> uv1;
        std::vector<Float> color;
        std::vector<UInt> indices;
        TopologyType topology{TopologyType::TriangleList};
        std::optional<AssetRef<MaterialAssetDesc>> material;
    };
}

#endif // DOLAS_MESH_ASSET_H

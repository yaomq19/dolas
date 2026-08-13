#ifndef DOLAS_MESH_ASSET_H
#define DOLAS_MESH_ASSET_H

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "asset_types/material_asset.h"
#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"
#include "dolas_base.h"

namespace Dolas
{
    enum class TopologyType : UInt
    {
        TriangleList = 0,
        TriangleStrip = 1,
    };

    template<>
    struct AssetEnumReflection<TopologyType>
    {
        [[nodiscard]] static constexpr auto GetValues() noexcept
        {
            return std::array<AssetEnumValue<TopologyType>, 2>{
                AssetEnumValue<TopologyType>{TopologyType::TriangleList, "TriangleList", "三角形列表", "triangleList"},
                AssetEnumValue<TopologyType>{TopologyType::TriangleStrip, "TriangleStrip", "三角形带", "triangleStrip"},
            };
        }
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

    template<>
    struct AssetReflection<MeshAssetDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = MeshAssetDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::position>("position"),
                MakeAssetField<2, &T::normal>("normal"),
                MakeAssetField<3, &T::tangent>("tangent"),
                MakeAssetField<4, &T::uv0>("uv0"),
                MakeAssetField<5, &T::uv1>("uv1"),
                MakeAssetField<6, &T::color>("color"),
                MakeAssetField<7, &T::indices>("indices"),
                MakeAssetField<8, &T::topology>("topology"),
                MakeAssetField<9, &T::material>("material"));
        }
    };
}

#endif // DOLAS_MESH_ASSET_H

#ifndef DOLAS_MATERIAL_ASSET_H
#define DOLAS_MATERIAL_ASSET_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"
#include "dolas_base.h"
#include "dolas_math.h"

namespace Dolas
{
    struct MaterialAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.material"};
        static constexpr std::string_view kFileSuffix{".material"};
        static constexpr std::uint32_t kSchemaVersion{1};

        std::optional<RawAssetRef> vertex_shader;
        std::optional<RawAssetRef> pixel_shader;
        std::map<std::string, Vector4> vertex_shader_global_variables;
        std::map<std::string, Vector4> pixel_shader_global_variables;
        std::map<std::string, RawAssetRef> pixel_shader_texture;
        std::map<std::string, Float> parameter;
    };

    template<>
    struct AssetReflection<MaterialAssetDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = MaterialAssetDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::vertex_shader>("vertex_shader"),
                MakeAssetField<2, &T::pixel_shader>("pixel_shader"),
                MakeAssetField<3, &T::vertex_shader_global_variables>("vertex_shader_global_variables"),
                MakeAssetField<4, &T::pixel_shader_global_variables>("pixel_shader_global_variables"),
                MakeAssetField<5, &T::pixel_shader_texture>("pixel_shader_texture"),
                MakeAssetField<6, &T::parameter>("parameter"));
        }
    };
}

#endif // DOLAS_MATERIAL_ASSET_H

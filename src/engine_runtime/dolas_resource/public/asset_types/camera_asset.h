#ifndef DOLAS_CAMERA_ASSET_H
#define DOLAS_CAMERA_ASSET_H

#include <array>
#include <cstdint>
#include <string_view>

#include "dolas_asset_schema.h"
#include "dolas_base.h"
#include "dolas_math.h"

namespace Dolas
{
    enum class CameraPerspectiveType : UInt
    {
        Perspective = 0,
        Orthographic = 1,
    };

    template<>
    struct AssetEnumReflection<CameraPerspectiveType>
    {
        [[nodiscard]] static constexpr auto GetValues() noexcept
        {
            return std::array<AssetEnumValue<CameraPerspectiveType>, 2>{
                AssetEnumValue<CameraPerspectiveType>{CameraPerspectiveType::Perspective, "Perspective", "透视", "perspective"},
                AssetEnumValue<CameraPerspectiveType>{CameraPerspectiveType::Orthographic, "Orthographic", "正交", "orthographic"},
            };
        }
    };

    struct CameraAssetDesc
    {
        static constexpr std::string_view kTypeId{"dolas.camera"};
        static constexpr std::string_view kFileSuffix{".camera"};
        static constexpr std::uint32_t kSchemaVersion{1};

        CameraPerspectiveType camera_perspective_type{CameraPerspectiveType::Perspective};
        Vector3 position{};
        Vector3 forward{0.0f, 0.0f, 1.0f};
        Vector3 up{0.0f, 1.0f, 0.0f};
        Float near_plane{0.1f};
        Float far_plane{2000.0f};
        Float fov{90.0f};
        Float aspect_ratio{1.77f};
        Float window_width{5.0f};
        Float window_height{5.0f};
    };

    template<>
    struct AssetReflection<CameraAssetDesc>
    {
        [[nodiscard]] static consteval auto GetSchema()
        {
            using T = CameraAssetDesc;
            return MakeAssetSchema<T>(
                MakeAssetField<1, &T::camera_perspective_type>("camera_perspective_type"),
                MakeAssetField<2, &T::position>("position", AssetFieldOptions{.required = true}),
                MakeAssetField<3, &T::forward>("forward", AssetFieldOptions{.required = true}),
                MakeAssetField<4, &T::up>("up", AssetFieldOptions{.required = true}),
                MakeAssetField<5, &T::near_plane>(
                    "near_plane",
                    AssetFieldOptions{.minimum = 0.0001, .display_name = "Near Plane"}),
                MakeAssetField<6, &T::far_plane>(
                    "far_plane",
                    AssetFieldOptions{.minimum = 0.0001, .display_name = "Far Plane"}),
                MakeAssetField<7, &T::fov>(
                    "fov",
                    AssetFieldOptions{.minimum = 1.0, .maximum = 179.0, .display_name = "Field of View"}),
                MakeAssetField<8, &T::aspect_ratio>(
                    "aspect_ratio",
                    AssetFieldOptions{.minimum = 0.0001, .display_name = "Aspect Ratio"}),
                MakeAssetField<9, &T::window_width>(
                    "window_width",
                    AssetFieldOptions{.minimum = 0.0001, .display_name = "Window Width"}),
                MakeAssetField<10, &T::window_height>(
                    "window_height",
                    AssetFieldOptions{.minimum = 0.0001, .display_name = "Window Height"}));
        }
    };
}

#endif // DOLAS_CAMERA_ASSET_H

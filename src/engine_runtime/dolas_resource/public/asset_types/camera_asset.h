#ifndef DOLAS_CAMERA_ASSET_H
#define DOLAS_CAMERA_ASSET_H

#include <cstdint>
#include <string_view>

#include "dolas_base.h"
#include "dolas_math.h"

namespace Dolas
{
    enum class CameraPerspectiveType : UInt
    {
        Perspective = 0,
        Orthographic = 1,
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
}

#endif // DOLAS_CAMERA_ASSET_H

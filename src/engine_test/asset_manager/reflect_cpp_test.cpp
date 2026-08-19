#include <catch2/catch_test_macros.hpp>
#include <rfl/json.hpp>

#include <string>

#include "asset_types/camera_asset.h"

namespace rfl
{
    template<>
    struct Reflector<Dolas::Vector3>
    {
        struct ReflType
        {
            Dolas::Float x;
            Dolas::Float y;
            Dolas::Float z;
        };

        [[nodiscard]] static Dolas::Vector3 to(const ReflType& value) noexcept
        {
            return {value.x, value.y, value.z};
        }

        [[nodiscard]] static ReflType from(const Dolas::Vector3& value) noexcept
        {
            return {value.x, value.y, value.z};
        }
    };
}

TEST_CASE("reflect-cpp round-trips a camera asset without field registration", "[Asset][Reflection]")
{
    Dolas::CameraAssetDesc source;
    source.camera_perspective_type = Dolas::CameraPerspectiveType::Orthographic;
    source.position = {1.0f, 2.0f, 3.0f};
    source.fov = 60.0f;

    const std::string json = rfl::json::write(source);
    const auto result = rfl::json::read<Dolas::CameraAssetDesc>(json);

    REQUIRE(result.has_value());
    const auto& camera = result.value();
    CHECK(camera.camera_perspective_type == source.camera_perspective_type);
    CHECK(camera.position.x == source.position.x);
    CHECK(camera.position.y == source.position.y);
    CHECK(camera.position.z == source.position.z);
    CHECK(camera.fov == source.fov);
}

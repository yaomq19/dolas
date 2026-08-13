#include <catch2/catch_test_macros.hpp>
#include "dolas_asset_path.h"
#include "dolas_asset_manager.h"
#include "dolas_paths.h"
#include "rsd/camera.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string_view>

using namespace Dolas;
namespace fs = std::filesystem;

namespace
{
#if !defined(NDEBUG)
    class EngineContentDirGuard
    {
    public:
        explicit EngineContentDirGuard(const fs::path& test_dir)
            : m_original_engine_dir(PathUtils::GetEngineContentDir())
            , m_test_dir(test_dir)
        {
            fs::create_directories(m_test_dir);
            PathUtils::SetEngineContentDirForDebug(m_test_dir.string());
        }

        ~EngineContentDirGuard()
        {
            PathUtils::SetEngineContentDirForDebug(m_original_engine_dir);
            std::error_code ec;
            fs::remove_all(m_test_dir, ec);
        }

    private:
        std::string m_original_engine_dir;
        fs::path m_test_dir;
    };
#endif

    fs::path MakeUniqueTestDir()
    {
        static std::atomic_uint32_t s_counter {0};
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return fs::current_path() / ("temp_test_assets_" + std::to_string(now) + "_" + std::to_string(s_counter.fetch_add(1)));
    }

    AssetPath RequireAssetPath(std::string_view value)
    {
        const auto asset_path = AssetPath::Parse(value);
        REQUIRE(asset_path.has_value());
        return *asset_path;
    }
}

// CameraRSD XML template for RSD parsing tests
// File suffix: .camera
// Vector3 fields (position, forward, up) are required — ParseFieldInto returns false
// for missing Vector3 elements. FloatValue fields can be omitted (defaults kept).
static const char* kCameraXmlTemplate = R"(<?xml version="1.0" encoding="UTF-8"?>
<root>
    <camera_perspective_type>%s</camera_perspective_type>
    <position x="0" y="0" z="0"/>
    <forward x="0" y="0" z="0"/>
    <up x="0" y="1" z="0"/>
</root>
)";

TEST_CASE("AssetManager ParseFieldInto - CameraRSD enum parsing", "[AssetManager][ParseField]")
{
#if !defined(NDEBUG)
    fs::path testDir = MakeUniqueTestDir();
    EngineContentDirGuard content_dir_guard{testDir};

    AssetManager manager;
    manager.Initialize();

    SECTION("Canonical name 'Perspective' parses to Perspective (0)")
    {
        fs::path cameraPath = testDir / "canonical.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "Perspective");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/canonical.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(camera->camera_perspective_type == CameraPerspectiveType::Perspective);
        REQUIRE(static_cast<UInt>(camera->camera_perspective_type) == 0);
    }

    SECTION("Display name (alias) 'perspective' parses to Perspective (0)")
    {
        fs::path cameraPath = testDir / "alias.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "perspective");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/alias.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(camera->camera_perspective_type == CameraPerspectiveType::Perspective);
    }

    SECTION("Case-insensitive matching 'PERSPECTIVE'")
    {
        fs::path cameraPath = testDir / "upper.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "PERSPECTIVE");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/upper.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(camera->camera_perspective_type == CameraPerspectiveType::Perspective);
    }

    SECTION("Numeric fallback '1' parses to Orthographic (1)")
    {
        fs::path cameraPath = testDir / "numeric.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "1");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/numeric.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(camera->camera_perspective_type == CameraPerspectiveType::Orthographic);
        REQUIRE(static_cast<UInt>(camera->camera_perspective_type) == 1);
    }

    SECTION("Canonical name 'Orthographic' parses to Orthographic (1)")
    {
        fs::path cameraPath = testDir / "ortho.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "Orthographic");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/ortho.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(camera->camera_perspective_type == CameraPerspectiveType::Orthographic);
    }

    SECTION("Missing enum element returns default value (0)")
    {
        // XML with no camera_perspective_type element - should get default
        // Must still include required Vector3 fields (position, forward, up)
        fs::path cameraPath = testDir / "default.camera";
        {
            std::ofstream ofs(cameraPath);
            ofs << R"(<?xml version="1.0" encoding="UTF-8"?>
<root>
    <position x="0" y="0" z="0"/>
    <forward x="0" y="0" z="0"/>
    <up x="0" y="1" z="0"/>
</root>
)";
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/default.camera"));
        REQUIRE(camera != nullptr);
        REQUIRE(static_cast<UInt>(camera->camera_perspective_type) == 0);
    }

    SECTION("Invalid enum string fails parsing")
    {
        fs::path cameraPath = testDir / "invalid_enum.camera";
        {
            std::ofstream ofs(cameraPath);
            char buf[512];
            snprintf(buf, sizeof(buf), kCameraXmlTemplate, "NonExistentValue");
            ofs << buf;
        }

        const CameraRSD* camera = manager.GetRsdAsset<CameraRSD>(RequireAssetPath("_engine/invalid_enum.camera"));
        REQUIRE(camera == nullptr);
    }

#else
    SUCCEED("Test skipped in Release build (debug-only functions not available)");
#endif
}

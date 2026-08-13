#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

#include "asset_types/camera_asset.h"
#include "asset_types/entity_asset.h"
#include "dolas_asset_manager.h"
#include "dolas_asset_path.h"
#include "dolas_paths.h"

using namespace Dolas;
namespace fs = std::filesystem;

namespace
{
    struct NotAnAssetDescription
    {
    };

    static_assert(AssetDescription<CameraAssetDesc>);
    static_assert(!AssetDescription<NotAnAssetDescription>);

#if !defined(NDEBUG)
    class ProjectContentDirGuard
    {
    public:
        explicit ProjectContentDirGuard(const fs::path& test_dir)
            : m_original_project_dir{PathUtils::GetProjectContentDir()}
            , m_test_dir{test_dir}
        {
            fs::create_directories(m_test_dir);
            PathUtils::SetProjectContentDirForDebug(m_test_dir.string());
        }

        ~ProjectContentDirGuard()
        {
            PathUtils::SetProjectContentDirForDebug(m_original_project_dir);
            std::error_code error;
            fs::remove_all(m_test_dir, error);
        }

    private:
        std::string m_original_project_dir;
        fs::path m_test_dir;
    };
#endif

    [[nodiscard]] fs::path MakeUniqueTestDir()
    {
        static std::atomic_uint32_t counter{0};
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return fs::current_path() / ("temp_test_assets_" + std::to_string(now) + "_" + std::to_string(counter.fetch_add(1)));
    }

    [[nodiscard]] AssetPath RequireAssetPath(std::string_view value)
    {
        const auto asset_path = AssetPath::Parse(value);
        REQUIRE(asset_path.has_value());
        return *asset_path;
    }

    void WriteCameraAsset(const fs::path& file_path, std::string_view perspective_type)
    {
        std::ofstream output{file_path};
        REQUIRE(output.is_open());
        output
            << "<asset type=\"dolas.camera\" version=\"1\">\n"
            << "    <camera_perspective_type>" << perspective_type << "</camera_perspective_type>\n"
            << "    <position x=\"0\" y=\"0\" z=\"0\"/>\n"
            << "    <forward x=\"0\" y=\"0\" z=\"1\"/>\n"
            << "    <up x=\"0\" y=\"1\" z=\"0\"/>\n"
            << "</asset>\n";
    }
}

TEST_CASE("AssetManager loads and caches typed C++ assets", "[AssetManager]")
{
#if !defined(NDEBUG)
    const fs::path test_dir = MakeUniqueTestDir();
    const ProjectContentDirGuard content_dir_guard{test_dir};

    AssetManager manager;
    REQUIRE(manager.Initialize());

    SECTION("Loads a valid project-mounted asset")
    {
        WriteCameraAsset(test_dir / "valid.camera", "Perspective");

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/valid.camera"));

        REQUIRE(load_result.HasValue());
        REQUIRE(load_result.GetError() == AssetLoadError::None);
        REQUIRE(load_result.GetAsset()->camera_perspective_type == CameraPerspectiveType::Perspective);
    }

    SECTION("Rejects a path whose suffix does not match the requested asset type")
    {
        WriteCameraAsset(test_dir / "wrong.scene", "Perspective");

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/wrong.scene"));

        REQUIRE_FALSE(load_result.HasValue());
        REQUIRE(load_result.GetAsset() == nullptr);
        REQUIRE(load_result.GetError() == AssetLoadError::FileSuffixMismatch);
    }

    SECTION("Reports an unavailable mount root")
    {
        PathUtils::SetProjectContentDirForDebug("");

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/unresolved.camera"));

        REQUIRE_FALSE(load_result.HasValue());
        REQUIRE(load_result.GetError() == AssetLoadError::PathResolutionFailed);
    }

    SECTION("Equivalent logical paths share one cache entry")
    {
        const fs::path file_path = test_dir / "cached.camera";
        WriteCameraAsset(file_path, "Perspective");

        const auto first_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/cached.camera"));
        WriteCameraAsset(file_path, "Orthographic");
        const auto second_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project//./cached.camera"));

        REQUIRE(first_result.HasValue());
        REQUIRE(second_result.HasValue());
        REQUIRE(first_result.GetAsset() == second_result.GetAsset());
        REQUIRE(second_result.GetAsset()->camera_perspective_type == CameraPerspectiveType::Perspective);
    }

    SECTION("A missing file is not cached as a failure")
    {
        const AssetPath asset_path = RequireAssetPath("_project/missing.camera");
        const auto missing_result = manager.LoadAsset<CameraAssetDesc>(asset_path);
        REQUIRE_FALSE(missing_result.HasValue());
        REQUIRE(missing_result.GetError() == AssetLoadError::FileReadFailed);

        WriteCameraAsset(test_dir / "missing.camera", "Perspective");
        const auto loaded_result = manager.LoadAsset<CameraAssetDesc>(asset_path);

        REQUIRE(loaded_result.HasValue());
        REQUIRE(loaded_result.GetAsset()->camera_perspective_type == CameraPerspectiveType::Perspective);
    }

    SECTION("Rejects malformed XML")
    {
        const fs::path file_path = test_dir / "malformed.camera";
        {
            std::ofstream output{file_path};
            REQUIRE(output.is_open());
            output << "This is not XML content";
        }

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/malformed.camera"));

        REQUIRE_FALSE(load_result.HasValue());
        REQUIRE(load_result.GetError() == AssetLoadError::XmlParseFailed);
    }

    SECTION("Rejects XML without a root element")
    {
        const fs::path file_path = test_dir / "no_root.camera";
        {
            std::ofstream output{file_path};
            REQUIRE(output.is_open());
            output << "<?xml version=\"1.0\"?><!-- no root element -->";
        }

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/no_root.camera"));

        REQUIRE_FALSE(load_result.HasValue());
        REQUIRE(load_result.GetError() == AssetLoadError::XmlRootMissing);
    }

    SECTION("Reports malformed asset fields")
    {
        WriteCameraAsset(test_dir / "invalid_field.camera", "NotACameraPerspective");

        const auto load_result = manager.LoadAsset<CameraAssetDesc>(RequireAssetPath("_project/invalid_field.camera"));

        REQUIRE_FALSE(load_result.HasValue());
        REQUIRE(load_result.GetError() == AssetLoadError::AssetFieldParseFailed);
    }

    SECTION("Clear discards cached assets")
    {
        const fs::path file_path = test_dir / "clear.camera";
        const AssetPath asset_path = RequireAssetPath("_project/clear.camera");
        WriteCameraAsset(file_path, "Perspective");

        const auto first_result = manager.LoadAsset<CameraAssetDesc>(asset_path);
        REQUIRE(first_result.HasValue());
        REQUIRE(first_result.GetAsset()->camera_perspective_type == CameraPerspectiveType::Perspective);

        WriteCameraAsset(file_path, "Orthographic");
        REQUIRE(manager.Clear());

        const auto reloaded_result = manager.LoadAsset<CameraAssetDesc>(asset_path);
        REQUIRE(reloaded_result.HasValue());
        REQUIRE(reloaded_result.GetAsset()->camera_perspective_type == CameraPerspectiveType::Orthographic);
    }
#else
    SUCCEED("Test skipped in Release builds because path root overrides are debug-only");
#endif
}

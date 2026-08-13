#include <catch2/catch_test_macros.hpp>
#include "dolas_paths.h"

#include <string_view>

using namespace Dolas;

namespace
{
#if !defined(NDEBUG)
    class PathRootsGuard
    {
    public:
        PathRootsGuard()
            : m_engine_root{PathUtils::GetEngineContentDir()}
            , m_project_root{PathUtils::GetProjectContentDir()}
        {
        }

        ~PathRootsGuard()
        {
            PathUtils::SetEngineContentDirForDebug(m_engine_root);
            PathUtils::SetProjectContentDirForDebug(m_project_root);
        }

    private:
        std::string m_engine_root;
        std::string m_project_root;
    };
#endif

    AssetPath RequireAssetPath(std::string_view value)
    {
        const auto asset_path = AssetPath::Parse(value);
        REQUIRE(asset_path.has_value());
        return *asset_path;
    }
}

TEST_CASE("PathUtils::ResolveAssetPath Tests", "[PathUtils]") {
#if !defined(NDEBUG)
    const PathRootsGuard roots_guard;

    // 设置一个已知的路径用于测试（故意带斜杠）
    PathUtils::SetEngineContentDirForDebug("C:/Engine/Content/");
    
    SECTION("Testing _engine prefix") {
        // 测试正常拼接（带斜杠）
        const auto result = PathUtils::ResolveAssetPath(RequireAssetPath("_engine/textures/stone.png"));
        REQUIRE(result.has_value());
        REQUIRE(result->generic_string() == "C:/Engine/Content/textures/stone.png"); // 应该没有双斜杠

        const auto normalized = PathUtils::ResolveAssetPath(RequireAssetPath("_engine\\textures//./stone.png/"));
        REQUIRE(normalized.has_value());
        REQUIRE(normalized->generic_string() == "C:/Engine/Content/textures/stone.png");
    }

    SECTION("Testing _project prefix") {
        PathUtils::SetProjectContentDirForDebug("D:/MyGame/Content");
        
        // 正常拼接
        const AssetPath project_asset_path = RequireAssetPath("_project/models/hero.fbx");
        const auto resultValid = PathUtils::ResolveAssetPath(project_asset_path);
        REQUIRE(resultValid.has_value());
        REQUIRE(resultValid->generic_string() == "D:/MyGame/Content/models/hero.fbx"); // 应该自动补全斜杠

        // 项目路径未设置的情况
        PathUtils::SetProjectContentDirForDebug("");
        const auto resultEmpty = PathUtils::ResolveAssetPath(project_asset_path);
        REQUIRE_FALSE(resultEmpty.has_value());
    }
#else
    // Release build - skip this test as debug-only functions are not available
    SUCCEED("Test skipped in Release build (debug-only functions not available)");
#endif
}

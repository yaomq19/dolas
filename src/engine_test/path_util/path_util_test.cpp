#include <catch2/catch_test_macros.hpp>
#include "dolas_paths.h"

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
}

TEST_CASE("PathUtils::CombineToFullPath Tests", "[PathUtils]") {
#if !defined(NDEBUG)
    const PathRootsGuard roots_guard;

    // 设置一个已知的路径用于测试（故意带斜杠）
    PathUtils::SetEngineContentDirForDebug("C:/Engine/Content/");
    
    SECTION("Testing _engine prefix") {
        // 测试正常拼接（带斜杠）
        auto result = PathUtils::CombineToFullPath("_engine/textures/stone.png");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "C:/Engine/Content/textures/stone.png"); // 应该没有双斜杠

        auto normalized = PathUtils::CombineToFullPath("_engine\\textures//./stone.png/");
        REQUIRE(normalized.has_value());
        REQUIRE(normalized.value() == "C:/Engine/Content/textures/stone.png");
    }

    SECTION("Testing _project prefix") {
        PathUtils::SetProjectContentDirForDebug("D:/MyGame/Content");
        
        // 正常拼接
        auto resultValid = PathUtils::CombineToFullPath("_project/models/hero.fbx");
        REQUIRE(resultValid.has_value());
        REQUIRE(resultValid.value() == "D:/MyGame/Content/models/hero.fbx"); // 应该自动补全斜杠

        // 项目路径未设置的情况
        PathUtils::SetProjectContentDirForDebug("");
        auto resultEmpty = PathUtils::CombineToFullPath("_project/models/hero.fbx");
        REQUIRE_FALSE(resultEmpty.has_value());
    }

    SECTION("Testing invalid prefixes") {
        REQUIRE_FALSE(PathUtils::CombineToFullPath("engine/text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("/_engine/text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("_engine").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("_engine/").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("_engine2/text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("_engine/../text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("_engine/C:/text.txt").has_value());
    }
#else
    // Release build - skip this test as debug-only functions are not available
    SUCCEED("Test skipped in Release build (debug-only functions not available)");
#endif
}

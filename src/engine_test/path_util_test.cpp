#include <catch2/catch_test_macros.hpp>
#include "dolas_paths.h"

using namespace Dolas;

TEST_CASE("PathUtils::CombineToFullPath Tests", "[PathUtils]") {
    // 准备测试环境
    std::string originalEngineDir = PathUtils::g_engine_content_directory_path;
    std::string originalProjectDir = PathUtils::g_project_content_directory_path;

    // 设置一个已知的路径用于测试
    PathUtils::g_engine_content_directory_path = "C:/Engine/Content/";
    
    SECTION("Testing _engine prefix") {
        auto result = PathUtils::CombineToFullPath("_engine/textures/stone.png");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "C:/Engine/Content/textures/stone.png");

        // 测试只有前缀的情况
        auto resultOnlyPrefix = PathUtils::CombineToFullPath("_engine");
        REQUIRE(resultOnlyPrefix.has_value());
        REQUIRE(resultOnlyPrefix.value() == "C:/Engine/Content/");
    }

    SECTION("Testing _project prefix") {
        // 初始状态下 project path 为空，应该返回 nullopt
        PathUtils::SetProjectDirectoryPath("");
        auto resultEmpty = PathUtils::CombineToFullPath("_project/models/hero.fbx");
        REQUIRE_FALSE(resultEmpty.has_value());

        // 设置 project path 后应该正常工作
        PathUtils::SetProjectDirectoryPath("D:/MyGame/Content/");
        auto resultValid = PathUtils::CombineToFullPath("_project/models/hero.fbx");
        REQUIRE(resultValid.has_value());
        REQUIRE(resultValid.value() == "D:/MyGame/Content/models/hero.fbx");

        // 测试只有前缀的情况
        auto resultOnlyPrefix = PathUtils::CombineToFullPath("_project");
        REQUIRE(resultOnlyPrefix.has_value());
        REQUIRE(resultOnlyPrefix.value() == "D:/MyGame/Content/");
    }

    SECTION("Testing invalid prefixes") {
        REQUIRE_FALSE(PathUtils::CombineToFullPath("engine/text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("/_engine/text.txt").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("random_path").has_value());
        REQUIRE_FALSE(PathUtils::CombineToFullPath("").has_value());
    }

    // 恢复原始状态，避免影响其他可能的测试
    PathUtils::g_engine_content_directory_path = originalEngineDir;
    PathUtils::g_project_content_directory_path = originalProjectDir;
}

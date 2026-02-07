#include <catch2/catch_test_macros.hpp>
#include "dolas_paths.h"

using namespace Dolas;

TEST_CASE("PathUtils::CombineToFullPath Tests", "[PathUtils]") {
    // 准备测试环境
    std::string originalEngineDir = PathUtils::GetEngineContentDir();
    std::string originalProjectDir = PathUtils::GetProjectContentDir();

    // 设置一个已知的路径用于测试（故意带斜杠）
    PathUtils::SetEngineContentDirForDebug("C:/Engine/Content/");
    
    SECTION("Testing _engine prefix") {
        // 测试正常拼接（带斜杠）
        auto result = PathUtils::CombineToFullPath("_engine/textures/stone.png");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "C:/Engine/Content/textures/stone.png"); // 应该没有双斜杠

        // 测试只有前缀（不带斜杠）
        auto resultOnlyPrefix = PathUtils::CombineToFullPath("_engine");
        REQUIRE(resultOnlyPrefix.has_value());
        REQUIRE(resultOnlyPrefix.value() == "C:/Engine/Content/");

        // 测试前缀带斜杠
        auto resultPrefixWithSlash = PathUtils::CombineToFullPath("_engine/");
        REQUIRE(resultPrefixWithSlash.has_value());
        REQUIRE(resultPrefixWithSlash.value() == "C:/Engine/Content/");
    }

    SECTION("Testing _project prefix") {
        PathUtils::SetProjectContentDirForDebug("D:/MyGame/Content"); // 故意不带斜杠
        
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
    }

    // 恢复原始状态
    PathUtils::SetEngineContentDirForDebug(originalEngineDir);
    PathUtils::SetProjectContentDirForDebug(originalProjectDir);
}

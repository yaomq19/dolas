#include <catch2/catch_test_macros.hpp>
#include "dolas_asset_manager.h"
#include "dolas_paths.h"
#include <filesystem>
#include <fstream>

using namespace Dolas;
namespace fs = std::filesystem;

TEST_CASE("AssetManagerNew::GetAsset Unit Tests", "[AssetManager]") {
    // 备份原始项目路径
    std::string originalProjectDir = PathUtils::GetProjectContentDir();
    
    // 创建临时的测试资产目录
    fs::path testDir = fs::current_path() / "temp_test_assets";
    if (fs::exists(testDir)) {
        fs::remove_all(testDir);
    }
    fs::create_directories(testDir);
    
    // 将项目内容目录重定向到临时目录
    PathUtils::SetProjectContentDirForDebug(testDir.string());

    AssetManagerNew manager;

    SECTION("加载有效的 XML 资产 (.ast)") {
        fs::path astPath = testDir / "valid_asset.ast";
        {
            std::ofstream ofs(astPath);
            ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root></root>";
        }

        const AssetBase* asset = manager.GetAsset("_project/valid_asset.ast");
        
        REQUIRE(asset != nullptr);
    }

    SECTION("缓存命中测试：多次请求同一资产应返回相同指针") {
        fs::path astPath = testDir / "cache_test.ast";
        {
            std::ofstream ofs(astPath);
            ofs << "<root></root>";
        }

        const AssetBase* firstCall = manager.GetAsset("_project/cache_test.ast");
        const AssetBase* secondCall = manager.GetAsset("_project/cache_test.ast");
        
        REQUIRE(firstCall != nullptr);
        REQUIRE(firstCall == secondCall);
    }

    SECTION("加载不存在的文件应返回 nullptr") {
        const AssetBase* asset = manager.GetAsset("_project/non_existent.ast");
        REQUIRE(asset == nullptr);
    }

    SECTION("加载无效的 XML 文件（格式错误）应返回 nullptr") {
        fs::path invalidAstPath = testDir / "invalid_xml.ast";
        {
            std::ofstream ofs(invalidAstPath);
            ofs << "This is not XML content";
        }

        const AssetBase* asset = manager.GetAsset("_project/invalid_xml.ast");
        REQUIRE(asset == nullptr);
    }

    SECTION("加载没有根元素的 XML 文件应返回 nullptr") {
        fs::path noRootPath = testDir / "no_root.ast";
        {
            std::ofstream ofs(noRootPath);
            ofs << ""; // 空文件
        }

        const AssetBase* asset = manager.GetAsset("_project/no_root.ast");
        REQUIRE(asset == nullptr);
    }

    SECTION("加载原始资产 (非 .ast 后缀) - 目前应返回 nullptr") {
        // 根据实现，目前 LoadRawAsset 总是返回 nullptr
        fs::path rawPath = testDir / "texture.png";
        {
            std::ofstream ofs(rawPath);
            ofs << "dummy binary data";
        }

        const AssetBase* asset = manager.GetAsset("_project/texture.png");
        REQUIRE(asset == nullptr);
    }

    SECTION("路径前缀无效应返回 nullptr") {
        const AssetBase* asset = manager.GetAsset("invalid_prefix/asset.ast");
        REQUIRE(asset == nullptr);
    }

    SECTION("空路径应返回 nullptr") {
        const AssetBase* asset = manager.GetAsset("");
        REQUIRE(asset == nullptr);
    }

    // 清理测试环境
    PathUtils::SetProjectContentDirForDebug(originalProjectDir);
    try {
        fs::remove_all(testDir);
    } catch (...) {
        // 忽略清理时的异常（例如文件被占用）
    }
}

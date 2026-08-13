#include <catch2/catch_test_macros.hpp>
#include "dolas_asset_path.h"
#include "dolas_asset_manager.h"
#include "dolas_paths.h"
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
    class ProjectContentDirGuard
    {
    public:
        ProjectContentDirGuard(const fs::path& test_dir)
            : m_original_project_dir(PathUtils::GetProjectContentDir())
            , m_test_dir(test_dir)
        {
            fs::create_directories(m_test_dir);
            PathUtils::SetProjectContentDirForDebug(m_test_dir.string());
        }

        ~ProjectContentDirGuard()
        {
            PathUtils::SetProjectContentDirForDebug(m_original_project_dir);
            std::error_code ec;
            fs::remove_all(m_test_dir, ec);
        }

    private:
        std::string m_original_project_dir;
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

TEST_CASE("AssetManagerNew::GetAsset Unit Tests", "[AssetManager]") {
#if !defined(NDEBUG)
    // 创建临时的测试资产目录
    fs::path testDir = MakeUniqueTestDir();
    ProjectContentDirGuard projectContentDirGuard(testDir);

    AssetManagerNew manager;

    SECTION("加载有效的 XML 资产 (.ast)") {
        fs::path astPath = testDir / "valid_asset.ast";
        {
            std::ofstream ofs(astPath);
            ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root></root>";
        }

        const AssetBase* asset = manager.GetAsset(RequireAssetPath("_project/valid_asset.ast"));
        
        REQUIRE(asset != nullptr);
    }

    SECTION("缓存命中测试：多次请求同一资产应返回相同指针") {
        fs::path astPath = testDir / "cache_test.ast";
        {
            std::ofstream ofs(astPath);
            ofs << "<root></root>";
        }

        const AssetBase* firstCall = manager.GetAsset(RequireAssetPath("_project/cache_test.ast"));
        const AssetBase* secondCall = manager.GetAsset(RequireAssetPath("_project//./cache_test.ast"));
        
        REQUIRE(firstCall != nullptr);
        REQUIRE(firstCall == secondCall);
    }

    SECTION("加载不存在的文件应返回 nullptr") {
        const AssetBase* asset = manager.GetAsset(RequireAssetPath("_project/non_existent.ast"));
        REQUIRE(asset == nullptr);
    }

    SECTION("加载无效的 XML 文件（格式错误）应返回 nullptr") {
        fs::path invalidAstPath = testDir / "invalid_xml.ast";
        {
            std::ofstream ofs(invalidAstPath);
            ofs << "This is not XML content";
        }

        const AssetBase* asset = manager.GetAsset(RequireAssetPath("_project/invalid_xml.ast"));
        REQUIRE(asset == nullptr);
    }

    SECTION("加载没有根元素的 XML 文件应返回 nullptr") {
        fs::path noRootPath = testDir / "no_root.ast";
        {
            std::ofstream ofs(noRootPath);
            ofs << ""; // 空文件
        }

        const AssetBase* asset = manager.GetAsset(RequireAssetPath("_project/no_root.ast"));
        REQUIRE(asset == nullptr);
    }

    SECTION("加载原始资产 (非 .ast 后缀) - 目前应返回 nullptr") {
        // 根据实现，目前 LoadRawAsset 总是返回 nullptr
        fs::path rawPath = testDir / "texture.png";
        {
            std::ofstream ofs(rawPath);
            ofs << "dummy binary data";
        }

        const AssetBase* asset = manager.GetAsset(RequireAssetPath("_project/texture.png"));
        REQUIRE(asset == nullptr);
    }
#else
    // Release build - skip this test as debug-only functions are not available
    SUCCEED("Test skipped in Release build (debug-only functions not available)");
#endif
}

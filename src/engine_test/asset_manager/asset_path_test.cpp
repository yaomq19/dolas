#include <catch2/catch_test_macros.hpp>

#include <string>
#include <unordered_set>

#include "dolas_asset_path.h"
#include "dolas_paths.h"

using namespace Dolas;

TEST_CASE("AssetPath parses and canonicalizes logical asset paths", "[AssetPath]")
{
    const auto engine_path = AssetPath::Parse("_engine\\textures//./stone.png/");
    REQUIRE(engine_path.has_value());
    REQUIRE(engine_path->GetMount() == AssetMount::Engine);
    REQUIRE(std::string{engine_path->GetRelativePath()} == "textures/stone.png");
    REQUIRE(engine_path->GetCanonicalPath() == "_engine/textures/stone.png");

    const auto project_path = AssetPath::Parse("_project/models/hero.fbx");
    REQUIRE(project_path.has_value());
    REQUIRE(project_path->GetMount() == AssetMount::Project);
    REQUIRE(std::string{project_path->GetRelativePath()} == "models/hero.fbx");
}

TEST_CASE("AssetPath rejects ambiguous or unsafe paths", "[AssetPath]")
{
    REQUIRE_FALSE(AssetPath::Parse("").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine/").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine2/texture.png").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_projectile/texture.png").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine/../outside.txt").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine/folder/../../outside.txt").has_value());
    REQUIRE_FALSE(AssetPath::Parse("_engine/C:/outside.txt").has_value());
    REQUIRE_FALSE(AssetPath::Parse("C:/outside.txt").has_value());
    REQUIRE_FALSE(AssetPath::Parse("/_engine/texture.png").has_value());
}

TEST_CASE("Equivalent AssetPath values compare and hash equally", "[AssetPath]")
{
    const auto first = AssetPath::Parse("_engine/materials/example.material");
    const auto second = AssetPath::Parse("_engine\\materials//./example.material");
    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    REQUIRE(*first == *second);

    std::unordered_set<AssetPath, AssetPathHash> paths;
    paths.insert(*first);
    REQUIRE(paths.contains(*second));
}

TEST_CASE("PathUtils resolves AssetPath within its selected content root", "[AssetPath][PathUtils]")
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

    const PathRootsGuard roots_guard;
    PathUtils::SetEngineContentDirForDebug("C:/Engine/Content/");
    PathUtils::SetProjectContentDirForDebug("D:/Game/Content");

    const auto engine_path = AssetPath::Parse("_engine/textures/stone.png");
    const auto project_path = AssetPath::Parse("_project/models/hero.fbx");
    REQUIRE(engine_path.has_value());
    REQUIRE(project_path.has_value());

    const auto resolved_engine_path = PathUtils::ResolveAssetPath(*engine_path);
    const auto resolved_project_path = PathUtils::ResolveAssetPath(*project_path);
    REQUIRE(resolved_engine_path.has_value());
    REQUIRE(resolved_project_path.has_value());
    REQUIRE(resolved_engine_path->generic_string() == "C:/Engine/Content/textures/stone.png");
    REQUIRE(resolved_project_path->generic_string() == "D:/Game/Content/models/hero.fbx");

    PathUtils::SetProjectContentDirForDebug("");
    REQUIRE_FALSE(PathUtils::ResolveAssetPath(*project_path).has_value());
#else
    SUCCEED("Root overrides are debug-only");
#endif
}

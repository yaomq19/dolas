#ifndef DOLAS_PATHS_H
#define DOLAS_PATHS_H

#include <filesystem>
#include <optional>
#include <string>

#include "dolas_asset_path.h"

namespace Dolas {
    class PathUtils
    {
    public:
        static void SetProjectDirectoryPath(const std::string& project_content_dir);
        static std::string GetEngineContentDir();
        static std::string GetProjectContentDir();
        [[nodiscard]] static std::optional<std::filesystem::path> ResolveAssetPath(const AssetPath& asset_path);
        static std::optional<std::string> CombineToFullPath(const std::string& relative_path);
        static std::string GetShadersSourceDir();

#if !defined(NDEBUG)
        static void SetEngineContentDirForDebug(const std::string& engine_content_dir);
        static void SetProjectContentDirForDebug(const std::string& project_content_dir);
#endif
        
    private:
        static std::string g_engine_content_directory_path;
        static std::string g_project_content_directory_path;
    };
} // namespace Dolas

#endif // DOLAS_PATHS_H

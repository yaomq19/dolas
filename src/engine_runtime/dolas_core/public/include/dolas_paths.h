#ifndef DOLAS_PATHS_H
#define DOLAS_PATHS_H
#include <optional>
#include <string>

namespace Dolas {
    class PathUtils
    {
    public:
        static std::string g_engine_content_directory_path;
        static std::string g_project_content_directory_path;
        static void SetProjectDirectoryPath(const std::string& project_content_dir);
        static std::string GetEngineContentDir();
        static std::string GetProjectContentDir();
        static std::optional<std::string> CombineToFullPath(const std::string& relative_path);
        static std::string GetShadersSourceDir();
    };
} // namespace Dolas

#endif // DOLAS_PATHS_H 
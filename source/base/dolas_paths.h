#ifndef DOLAS_PATHS_H
#define DOLAS_PATHS_H
#include <string>

namespace Dolas {
// 根目录
#ifndef CMAKE_SOURCE_DIR
#define CMAKE_SOURCE_DIR "."
#endif

// 运行时输出目录，即 build/bin 目录
#ifndef CMAKE_RUNTIME_OUTPUT_DIRECTORY
#define CMAKE_RUNTIME_OUTPUT_DIRECTORY "."
#endif

    class PathUtils
    {
    public:
        static std::string GetContentDir();
        static std::string GetShadersSourceDir();
        static std::string GetShadersCompiledDir();
        static std::string GetEntityDir();
        static std::string GetMeshDir();
        static std::string GetMaterialDir();
        static std::string GetTextureDir();
        static std::string GetCameraDir();
    private:
        static std::string GetProjectRoot();
        static std::string GetBinDir();
    };
} // namespace Dolas

#endif // DOLAS_PATHS_H 
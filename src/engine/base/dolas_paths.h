#ifndef DOLAS_PATHS_H
#define DOLAS_PATHS_H
#include <string>

namespace Dolas {
    class PathUtils
    {
    public:
        static std::string GetContentDir();
        static std::string GetShadersSourceDir();
    };
} // namespace Dolas

#endif // DOLAS_PATHS_H 
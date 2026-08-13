#ifndef DOLAS_STRING_UTIL_H
#define DOLAS_STRING_UTIL_H

#include <string>

namespace Dolas
{
    class StringUtil
    {
    public:
        static std::wstring StringToWString(const std::string& str);
        static std::string WStringToString(const std::wstring& wstr);
    };
}
#endif // DOLAS_STRING_UTIL_H
#include "dolas_string_util.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <codecvt>
#include <locale>
#endif

namespace Dolas
{
    std::wstring StringUtil::StringToWString(const std::string& str)
    {
        if (str.empty())
            return std::wstring();

#ifdef _WIN32
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
        if (size_needed == 0)
            return std::wstring();
        
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
        return wstr;
#else
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.from_bytes(str);
        }
        catch (const std::range_error&)
        {
            return std::wstring();
        }
#endif
    }

    std::string StringUtil::WStringToString(const std::wstring& wstr)
    {
        if (wstr.empty())
            return std::string();

#ifdef _WIN32
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        if (size_needed == 0)
            return std::string();
        
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
        return str;
#else
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.to_bytes(wstr);
        }
        catch (const std::range_error&)
        {
            return std::string();
        }
#endif
    }
} // namespace Dolas

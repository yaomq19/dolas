#ifndef DOLAS_STRING_UTILS_H
#define DOLAS_STRING_UTILS_H

#include <string>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Dolas {
namespace StringUtils {

    /**
     * @brief 将std::string转换为std::wstring（Windows API方法）
     * @param str 输入的窄字符串
     * @param codePage 代码页（默认CP_UTF8）
     * @return 转换后的宽字符串
     */
    inline std::wstring StringToWString(const std::string& str, UINT codePage = CP_UTF8) {
        if (str.empty()) {
            return std::wstring();
        }

#ifdef _WIN32
        // 使用Windows API进行转换
        int size_needed = MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
        if (size_needed <= 0) {
            return std::wstring();
        }

        std::wstring result(size_needed, 0);
        MultiByteToWideChar(codePage, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size_needed);
        return result;
#else
        // 非Windows平台使用codecvt
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.from_bytes(str);
        } catch (const std::exception&) {
            return std::wstring();
        }
#endif
    }

    /**
     * @brief 将std::wstring转换为std::string（Windows API方法）
     * @param wstr 输入的宽字符串
     * @param codePage 代码页（默认CP_UTF8）
     * @return 转换后的窄字符串
     */
    inline std::string WStringToString(const std::wstring& wstr, UINT codePage = CP_UTF8) {
        if (wstr.empty()) {
            return std::string();
        }

#ifdef _WIN32
        // 使用Windows API进行转换
        int size_needed = WideCharToMultiByte(codePage, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
        if (size_needed <= 0) {
            return std::string();
        }

        std::string result(size_needed, 0);
        WideCharToMultiByte(codePage, 0, wstr.c_str(), static_cast<int>(wstr.size()), &result[0], size_needed, NULL, NULL);
        return result;
#else
        // 非Windows平台使用codecvt
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.to_bytes(wstr);
        } catch (const std::exception&) {
            return std::string();
        }
#endif
    }

    /**
     * @brief 将std::string转换为std::wstring（locale方法）
     * @param str 输入的窄字符串
     * @param localeName 区域设置名称（如"zh_CN.UTF-8"）
     * @return 转换后的宽字符串
     */
    inline std::wstring StringToWStringLocale(const std::string& str, const std::string& localeName = "") {
        if (str.empty()) {
            return std::wstring();
        }

        try {
            // 设置locale
            std::locale loc = localeName.empty() ? std::locale("") : std::locale(localeName);
            
            // 使用locale进行转换
            std::wstring result;
            result.reserve(str.size());
            
            const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(loc);
            
            for (char c : str) {
                result.push_back(facet.widen(c));
            }
            
            return result;
        } catch (const std::exception&) {
            // 如果locale转换失败，回退到简单转换
            return std::wstring(str.begin(), str.end());
        }
    }

    /**
     * @brief 将std::string转换为std::wstring（简单方法，仅适用于ASCII）
     * @param str 输入的窄字符串
     * @return 转换后的宽字符串
     */
    inline std::wstring StringToWStringSimple(const std::string& str) {
        return std::wstring(str.begin(), str.end());
    }

    /**
     * @brief 将std::wstring转换为std::string（简单方法，仅适用于ASCII）
     * @param wstr 输入的宽字符串
     * @return 转换后的窄字符串
     */
    inline std::string WStringToStringSimple(const std::wstring& wstr) {
        return std::string(wstr.begin(), wstr.end());
    }

    /**
     * @brief 获取字符串的UTF-8字节长度
     * @param str 输入字符串
     * @return UTF-8字节长度
     */
    inline size_t GetUTF8ByteLength(const std::string& str) {
        return str.length();
    }

    /**
     * @brief 获取宽字符串的字符数量
     * @param wstr 输入宽字符串
     * @return 字符数量
     */
    inline size_t GetWStringCharCount(const std::wstring& wstr) {
        return wstr.length();
    }

    /**
     * @brief 检查字符串是否为有效的UTF-8
     * @param str 输入字符串
     * @return 如果是有效UTF-8返回true
     */
    inline bool IsValidUTF8(const std::string& str) {
        try {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            converter.from_bytes(str);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    /**
     * @brief 安全的字符串转换，自动选择最佳方法
     * @param str 输入的窄字符串
     * @return 转换后的宽字符串
     */
    inline std::wstring SafeStringToWString(const std::string& str) {
        if (str.empty()) {
            return std::wstring();
        }

        // 首先尝试UTF-8转换
        std::wstring result = StringToWString(str, CP_UTF8);
        if (!result.empty()) {
            return result;
        }

        // 如果UTF-8失败，尝试本地代码页
#ifdef _WIN32
        result = StringToWString(str, CP_ACP);
        if (!result.empty()) {
            return result;
        }
#endif

        // 最后回退到简单转换
        return StringToWStringSimple(str);
    }

    // =============== 使用示例 ===============
    /*
     * // 基本转换
     * std::string narrow = "Hello World";
     * std::wstring wide = Dolas::StringUtils::StringToWString(narrow);
     * 
     * // 中文字符串转换
     * std::string chinese = "你好世界";
     * std::wstring wideChina = Dolas::StringUtils::StringToWString(chinese, CP_UTF8);
     * 
     * // 安全转换（推荐）
     * std::string anyString = "Mixed 字符串 content";
     * std::wstring wideSafe = Dolas::StringUtils::SafeStringToWString(anyString);
     * 
     * // 反向转换
     * std::string backToNarrow = Dolas::StringUtils::WStringToString(wideSafe);
     * 
     * // 用于DirectX
     * std::string filePath = "textures/hero.dds";
     * std::wstring wideFilePath = Dolas::StringUtils::StringToWString(filePath);
     * DirectX::CreateDDSTextureFromFile(device, wideFilePath.c_str(), nullptr, &srv);
     */

} // namespace StringUtils
} // namespace Dolas

#endif // DOLAS_STRING_UTILS_H 
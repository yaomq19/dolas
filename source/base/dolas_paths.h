#ifndef DOLAS_PATHS_H
#define DOLAS_PATHS_H

#include <string>
#include <filesystem>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Dolas {
namespace Paths {

    // 使用CMake传递的宏定义
    #ifndef PROJECT_ROOT_DIR
    #define PROJECT_ROOT_DIR "."
    #endif

    #ifndef PROJECT_NAME
    #define PROJECT_NAME "Dolas"
    #endif

    #ifndef PROJECT_VERSION
    #define PROJECT_VERSION "0.1.0"
    #endif

#ifndef CMAKE_RUNTIME_OUTPUT_DIRECTORY
#define CMAKE_RUNTIME_OUTPUT_DIRECTORY "."
#endif
    
    /**
     * @brief 将std::string转换为std::wstring
     * @param str 输入的窄字符串
     * @return 转换后的宽字符串
     */
    inline std::wstring StringToWString(const std::string& str) {
        if (str.empty()) return std::wstring();
        
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    /**
     * @brief 将路径字符串转换为wchar_t指针（注意：返回的指针指向临时对象，需要立即使用）
     * @param path 路径字符串
     * @return wchar_t指针，指向转换后的宽字符串
     * @warning 返回的指针指向临时对象，必须立即使用，不能存储！
     */
    inline const wchar_t* PathToWChar(const std::string& path) {
        static thread_local std::wstring temp_wstring;
        temp_wstring = StringToWString(path);
        return temp_wstring.c_str();
    }

    /**
     * @brief 获取项目根目录路径
     * @return 项目根目录的绝对路径
     */
    inline std::string GetProjectRoot() {
        return std::string(PROJECT_ROOT_DIR);
    }

	inline std::string GetBinDir() {
		return std::string(CMAKE_RUNTIME_OUTPUT_DIRECTORY);
	}

    /**
     * @brief 获取着色器目录路径
     * @return 着色器源码目录的绝对路径
     */
    inline std::string GetShadersDir() {
        return GetProjectRoot() + "/shaders";
    }

    /**
     * @brief 获取编译后着色器目录路径
     * @return 编译后着色器文件的目录路径
     */
    inline std::string GetCompiledShadersDir() {
        return GetProjectRoot() + "/build/bin/shaders";
    }

    /**
     * @brief 获取内容资源目录路径
     * @return 内容资源目录的绝对路径
     */
    inline std::string GetContentDir() {
        return GetProjectRoot() + "/content";
    }

    /**
     * @brief 获取配置文件目录路径
     * @return 配置文件目录的绝对路径
     */
    inline std::string GetConfigDir() {
        return GetProjectRoot() + "/config";
    }

    /**
     * @brief 获取第三方库目录路径
     * @return 第三方库目录的绝对路径
     */
    inline std::string GetThirdPartyDir() {
        return GetProjectRoot() + "/third_party";
    }

    /**
     * @brief 获取项目名称
     * @return 项目名称字符串
     */
    inline std::string GetProjectName() {
        return std::string(PROJECT_NAME);
    }

    /**
     * @brief 获取项目版本
     * @return 项目版本字符串
     */
    inline std::string GetProjectVersion() {
        return std::string(PROJECT_VERSION);
    }

    /**
     * @brief 获取项目根目录路径（宽字符版本）
     * @return 项目根目录的宽字符串
     */
    inline std::wstring GetProjectRootW() {
        return StringToWString(GetProjectRoot());
    }

    /**
     * @brief 获取着色器目录路径（宽字符版本）
     * @return 着色器源码目录的宽字符串
     */
    inline std::wstring GetShadersDirW() {
        return StringToWString(GetShadersDir());
    }

    /**
     * @brief 获取编译后着色器目录路径（宽字符版本）
     * @return 编译后着色器文件的目录宽字符串
     */
    inline std::wstring GetCompiledShadersDirW() {
        return StringToWString(GetCompiledShadersDir());
    }

    /**
     * @brief 获取内容资源目录路径（宽字符版本）
     * @return 内容资源目录的宽字符串
     */
    inline std::wstring GetContentDirW() {
        return StringToWString(GetContentDir());
    }

    /**
     * @brief 构建相对于项目根目录的完整路径
     * @param relativePath 相对路径
     * @return 完整的绝对路径
     */
    inline std::string BuildPath(const std::string& relativePath) {
        std::filesystem::path rootPath(GetProjectRoot());
        std::filesystem::path fullPath = rootPath / relativePath;
        return fullPath.string();
    }

    /**
     * @brief 构建相对于项目根目录的完整路径（宽字符版本）
     * @param relativePath 相对路径
     * @return 完整的绝对路径宽字符串
     */
    inline std::wstring BuildPathW(const std::string& relativePath) {
        return StringToWString(BuildPath(relativePath));
    }

    /**
     * @brief 构建内容目录下的完整路径
     * @param relativePath 相对于content目录的路径
     * @return 完整的绝对路径字符串
     */
    inline std::string BuildContentPath(const std::string& relativePath) {
        return GetContentDir() + "/" + relativePath;
    }

    /**
     * @brief 构建内容目录下的完整路径（宽字符版本）
     * @param relativePath 相对于content目录的路径
     * @return 完整的绝对路径宽字符串
     */
    inline std::wstring BuildContentPathW(const std::string& relativePath) {
        return StringToWString(BuildContentPath(relativePath));
    }

    /**
     * @brief 检查文件是否存在
     * @param filePath 文件路径（可以是相对路径或绝对路径）
     * @return 如果文件存在返回true，否则返回false
     */
    inline bool FileExists(const std::string& filePath) {
        // 如果是相对路径，则相对于项目根目录
        std::filesystem::path path(filePath);
        if (path.is_relative()) {
            path = std::filesystem::path(GetProjectRoot()) / path;
        }
        return std::filesystem::exists(path);
    }

    /**
     * @brief 打印所有路径信息（用于调试）
     */
    inline void PrintPathInfo() {
        printf("=== Dolas Engine Path Information ===\n");
        printf("Project Name: %s\n", GetProjectName().c_str());
        printf("Project Version: %s\n", GetProjectVersion().c_str());
        printf("Project Root: %s\n", GetProjectRoot().c_str());
        printf("Shaders Dir: %s\n", GetShadersDir().c_str());
        printf("Compiled Shaders Dir: %s\n", GetCompiledShadersDir().c_str());
        printf("Content Dir: %s\n", GetContentDir().c_str());
        printf("Config Dir: %s\n", GetConfigDir().c_str());
        printf("Third Party Dir: %s\n", GetThirdPartyDir().c_str());
        printf("=====================================\n");
    }

    /* 
     * =============== 使用示例 ===============
     * 
     * // 1. 获取普通字符串路径
     * std::string contentPath = Dolas::Paths::BuildContentPath("textures/test.dds");
     * 
     * // 2. 转换为宽字符串用于DirectX API
     * std::wstring wContentPath = Dolas::Paths::BuildContentPathW("textures/test.dds");
     * 
     * // 3. 直接获取wchar_t*指针（立即使用）
     * DirectX::CreateDDSTextureFromFile(
     *     device, 
     *     Dolas::Paths::PathToWChar(Dola::Paths::BuildContentPath("textures/test.dds")), 
     *     nullptr, 
     *     &textureResourceView
     * );
     * 
     * // 4. 或者更简洁的方式
     * auto texturePath = Dolas::Paths::BuildContentPathW("textures/test.dds");
     * DirectX::CreateDDSTextureFromFile(device, texturePath.c_str(), nullptr, &srv);
     * 
     * ==========================================
     */

} // namespace Paths
} // namespace Dolas

#endif // DOLAS_PATHS_H 
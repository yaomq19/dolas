#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

class FileUtils {
public:
    // 获取目录下所有指定扩展名的文件
    static std::vector<std::string> GetFilesWithExtension(const std::string& directory, const std::string& extension);
    
    // 检查文件是否存在
    static bool FileExists(const std::string& filepath);
    
    // 检查目录是否存在
    static bool DirectoryExists(const std::string& directory);
    
    // 创建目录
    static bool CreateDirectory(const std::string& directory);
    
    // 获取文件名（不包含路径）
    static std::string GetFilename(const std::string& filepath);
    
    // 获取文件扩展名
    static std::string GetFileExtension(const std::string& filepath);
    
    // 获取文件名（不包含扩展名）
    static std::string GetFilenameWithoutExtension(const std::string& filepath);
    
    // 读取文件内容
    static std::string ReadFileContent(const std::string& filepath);
    
    // 获取文件大小
    static size_t GetFileSize(const std::string& filepath);
    
    // 规范化路径
    static std::string NormalizePath(const std::string& path);
    
    // 组合路径
    static std::string JoinPath(const std::string& path1, const std::string& path2);
};

#endif // FILE_UTILS_H

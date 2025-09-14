#include "file_utils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

std::vector<std::string> FileUtils::GetFilesWithExtension(const std::string& directory, const std::string& extension) {
    std::vector<std::string> files;
    
    try {
        // 首先规范化目录路径
        fs::path dir_path = fs::path(directory).lexically_normal();
        
        if (!fs::exists(dir_path)) {
            // 目录不存在
            return files;
        }
        
        if (!fs::is_directory(dir_path)) {
            // 不是目录
            return files;
        }
        
        for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                std::string file_ext = GetFileExtension(filepath);
                
                // 转换为小写进行比较
                std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);
                std::string target_ext = extension;
                std::transform(target_ext.begin(), target_ext.end(), target_ext.begin(), ::tolower);
                
                if (file_ext == target_ext) {
                    files.push_back(NormalizePath(filepath));
                }
            }
        }
        
        // 按文件名排序
        std::sort(files.begin(), files.end());
        
    } catch (const fs::filesystem_error& e) {
        // 静默处理文件系统错误，但在调试模式下可以输出
        // std::cerr << "Filesystem error in GetFilesWithExtension: " << e.what() << std::endl;
    }
    
    return files;
}

bool FileUtils::FileExists(const std::string& filepath) {
    try {
        return fs::exists(filepath) && fs::is_regular_file(filepath);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

bool FileUtils::DirectoryExists(const std::string& directory) {
    try {
        return fs::exists(directory) && fs::is_directory(directory);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

bool FileUtils::CreateDirectory(const std::string& directory) {
    try {
        return fs::create_directories(directory);
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

std::string FileUtils::GetFilename(const std::string& filepath) {
    try {
        fs::path path(filepath);
        return path.filename().string();
    } catch (const fs::filesystem_error&) {
        // 如果文件系统操作失败，手动解析
        size_t pos = filepath.find_last_of("/\\");
        if (pos != std::string::npos) {
            return filepath.substr(pos + 1);
        }
        return filepath;
    }
}

std::string FileUtils::GetFileExtension(const std::string& filepath) {
    try {
        fs::path path(filepath);
        std::string ext = path.extension().string();
        
        // 检查是否是复合扩展名（如 .vs.hlsl）
        std::string stem = path.stem().string();
        if (!stem.empty()) {
            size_t dot_pos = stem.find_last_of('.');
            if (dot_pos != std::string::npos) {
                // 返回复合扩展名
                return stem.substr(dot_pos) + ext;
            }
        }
        
        return ext;
    } catch (const fs::filesystem_error&) {
        // 手动解析扩展名
        size_t pos = filepath.find_last_of('.');
        if (pos != std::string::npos) {
            return filepath.substr(pos);
        }
        return "";
    }
}

std::string FileUtils::GetFilenameWithoutExtension(const std::string& filepath) {
    try {
        fs::path path(filepath);
        return path.stem().string();
    } catch (const fs::filesystem_error&) {
        // 手动解析
        std::string filename = GetFilename(filepath);
        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            return filename.substr(0, pos);
        }
        return filename;
    }
}

std::string FileUtils::ReadFileContent(const std::string& filepath) {
    try {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return "";
        }
        
        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
        
    } catch (const std::exception&) {
        return "";
    }
}

size_t FileUtils::GetFileSize(const std::string& filepath) {
    try {
        return fs::file_size(filepath);
    } catch (const fs::filesystem_error&) {
        return 0;
    }
}

std::string FileUtils::NormalizePath(const std::string& path) {
    try {
        fs::path normalized = fs::path(path).lexically_normal();
        return normalized.string();
    } catch (const fs::filesystem_error&) {
        // 简单的路径规范化
        std::string result = path;
        
        // 替换反斜杠为正斜杠
        std::replace(result.begin(), result.end(), '\\', '/');
        
        // 移除重复的斜杠
        size_t pos = 0;
        while ((pos = result.find("//", pos)) != std::string::npos) {
            result.replace(pos, 2, "/");
        }
        
        return result;
    }
}

std::string FileUtils::JoinPath(const std::string& path1, const std::string& path2) {
    try {
        fs::path result = fs::path(path1) / fs::path(path2);
        return result.string();
    } catch (const fs::filesystem_error&) {
        // 手动拼接路径
        std::string result = path1;
        
        if (!result.empty() && result.back() != '/' && result.back() != '\\') {
            result += "/";
        }
        
        std::string second = path2;
        if (!second.empty() && (second.front() == '/' || second.front() == '\\')) {
            second = second.substr(1);
        }
        
        result += second;
        return NormalizePath(result);
    }
}

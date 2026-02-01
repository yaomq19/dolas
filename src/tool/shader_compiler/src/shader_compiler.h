#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

struct CompilationResult {
    std::string filename;
    bool success;
    std::string error_message;
    std::string bytecode_size;
    double compilation_time_ms;
    std::string shader_model;
    std::string entry_point;
};

class ShaderCompiler {
public:
    ShaderCompiler();
    ~ShaderCompiler();

    // 编译单个shader文件
    CompilationResult CompileShader(const std::string& filepath, const std::string& entry_point = "", const std::string& target = "");
    
    // 编译目录下所有shader文件
    std::vector<CompilationResult> CompileAllShaders(const std::string& directory);
    
    // 编译指定的shader文件列表
    std::vector<CompilationResult> CompileShaderList(const std::vector<std::string>& filepaths);
    
    // 设置包含目录
    void SetIncludeDirectory(const std::string& include_dir);
    
    // 获取编译统计信息
    void PrintCompilationSummary(const std::vector<CompilationResult>& results);

private:
    std::string m_include_directory;
    std::map<std::string, std::string> m_shader_targets; // 文件扩展名到目标的映射
    
    // 根据文件名推断shader类型和入口点
    std::pair<std::string, std::string> InferShaderTypeAndEntry(const std::string& filename);
    
    // Windows特定的D3D编译实现
#ifdef _WIN32
    CompilationResult CompileHLSLShader(const std::string& filepath, const std::string& entry_point, const std::string& target);
#endif
};

#endif // SHADER_COMPILER_H

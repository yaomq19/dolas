#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include "shader_compiler.h"
#include "logger.h"
#include "file_utils.h"

void PrintUsage(const std::string& program_name) {
    std::cout << "Dolas Shader Compiler - Continuous Shader File Validity Monitoring Tool" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << program_name << "                    # Compile all shader files" << std::endl;
    std::cout << "  " << program_name << " <file1> [file2...] # Compile specified files" << std::endl;
    std::cout << "  " << program_name << " --help             # Show help information" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  - Default search directory: content/shader/" << std::endl;
    std::cout << "  - Supported file types: .hlsl" << std::endl;
    std::cout << "  - Log output: Console + logs/ directory" << std::endl;
    std::cout << "  - Error details: logs/compilation_errors.log" << std::endl;
}

void PrintHeader() {
    std::cout << "=====================================================" << std::endl;
    std::cout << "    Dolas Shader Compiler v1.0" << std::endl;
    std::cout << "    Continuous Shader File Compilation Validity Tool" << std::endl;
    std::cout << "=====================================================" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    PrintHeader();
    
    // 检查帮助参数
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }
    
    // 初始化日志系统
    try {
        auto current_time = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(current_time);
        
#ifdef LOG_OUTPUT_DIR
        std::string log_dir = LOG_OUTPUT_DIR;
#else
        std::string log_dir = "logs";
#endif
        
        if (!FileUtils::DirectoryExists(log_dir)) {
            FileUtils::CreateDirectory(log_dir);
        }
        
        Logger::Instance().Initialize(log_dir);
        Logger::Instance().SetLogLevel(LogLevel::DEBUG);
        
        LOG_INFO("Shader compiler started");
        LOG_INFO("Log directory: " + log_dir);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: Unable to initialize log system: " << e.what() << std::endl;
        return 1;
    }
    
    // 设置shader目录
#ifdef SHADER_CONTENT_DIR
    std::string shader_dir = SHADER_CONTENT_DIR;
#else
    std::string shader_dir = "content/shader";
#endif
    
    // 规范化路径
    shader_dir = FileUtils::NormalizePath(shader_dir);
    
    std::string raw_shader_dir;
#ifdef SHADER_CONTENT_DIR
    raw_shader_dir = SHADER_CONTENT_DIR;
#else
    raw_shader_dir = "content/shader";
#endif
    LOG_DEBUG("Raw shader directory: " + raw_shader_dir);
    LOG_DEBUG("Normalized shader directory: " + shader_dir);
    
    if (!FileUtils::DirectoryExists(shader_dir)) {
        LOG_ERROR("Shader directory does not exist: " + shader_dir);
        std::cerr << "Error: Shader directory does not exist: " << shader_dir << std::endl;
        return 1;
    }
    
    LOG_INFO("Shader search directory: " + shader_dir);
    std::cout << "Shader search directory: " << shader_dir << std::endl;
    
    // 初始化编译器
    ShaderCompiler compiler;
    compiler.SetIncludeDirectory(shader_dir);
    
    std::vector<CompilationResult> results;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        if (argc == 1) {
            // 没有参数，编译所有shader文件
            LOG_INFO("Starting to compile all shader files...");
            std::cout << "Scanning and compiling all .hlsl files..." << std::endl;
            
            results = compiler.CompileAllShaders(shader_dir);
            
        } else {
            // 有参数，编译指定文件
            std::vector<std::string> files_to_compile;
            
            for (int i = 1; i < argc; ++i) {
                std::string filepath = argv[i];
                
                // 如果是相对路径，添加shader目录前缀
                if (filepath.find('/') == std::string::npos && filepath.find('\\') == std::string::npos) {
                    filepath = FileUtils::JoinPath(shader_dir, filepath);
                }
                
                if (FileUtils::FileExists(filepath)) {
                    files_to_compile.push_back(filepath);
                    LOG_INFO("Adding file for compilation: " + filepath);
                } else {
                    LOG_WARNING("File does not exist: " + filepath);
                    std::cout << "Warning: File does not exist: " << filepath << std::endl;
                }
            }
            
            if (!files_to_compile.empty()) {
                LOG_INFO("Starting to compile " + std::to_string(files_to_compile.size()) + " specified files...");
                std::cout << "Compiling " << files_to_compile.size() << " specified files..." << std::endl;
                
                results = compiler.CompileShaderList(files_to_compile);
            } else {
                LOG_ERROR("No valid files to compile");
                std::cerr << "Error: No valid files to compile" << std::endl;
                return 1;
            }
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception occurred during compilation: " + std::string(e.what()));
        std::cerr << "Error: Exception occurred during compilation: " << e.what() << std::endl;
        return 1;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 输出编译结果摘要
    std::cout << std::endl;
    std::cout << "=====================================================" << std::endl;
    std::cout << "               Compilation Summary" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    compiler.PrintCompilationSummary(results);
    
    // 统计信息
    int successful = 0;
    int failed = 0;
    for (const auto& result : results) {
        if (result.success) {
            successful++;
        } else {
            failed++;
        }
    }
    
    std::cout << std::endl;
    std::cout << "Total: " << results.size() << " files" << std::endl;
    std::cout << "Success: " << successful << " files" << std::endl;
    std::cout << "Failed: " << failed << " files" << std::endl;
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    
    LOG_INFO("Compilation completed - Success: " + std::to_string(successful) + ", Failed: " + std::to_string(failed));
    LOG_INFO("Total time: " + std::to_string(duration.count()) + " ms");
    
    // 刷新日志
    Logger::Instance().Flush();
    
    std::cout << std::endl;
    if (failed > 0) {
        std::cout << "Note: Check logs/compilation_errors.log for detailed error information" << std::endl;
        return 1;
    } else {
        std::cout << "All shader files compiled successfully! yes" << std::endl;
        return 0;
    }
}

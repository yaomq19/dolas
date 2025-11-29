#include "shader_compiler.h"
#include "logger.h"
#include "file_utils.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <d3dcompiler.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#pragma comment(lib, "d3dcompiler.lib")
#endif

ShaderCompiler::ShaderCompiler() {
    // 初始化shader目标映射
    m_shader_targets[".vs.hlsl"] = "vs_5_0";      // Vertex Shader
    m_shader_targets[".ps.hlsl"] = "ps_5_0";      // Pixel Shader
    m_shader_targets[".gs.hlsl"] = "gs_5_0";      // Geometry Shader
    m_shader_targets[".hs.hlsl"] = "hs_5_0";      // Hull Shader
    m_shader_targets[".ds.hlsl"] = "ds_5_0";      // Domain Shader
    m_shader_targets[".cs.hlsl"] = "cs_5_0";      // Compute Shader
}

ShaderCompiler::~ShaderCompiler() = default;

void ShaderCompiler::SetIncludeDirectory(const std::string& include_dir) {
    m_include_directory = include_dir;
    LOG_DEBUG("Setting include directory: " + include_dir);
}

std::pair<std::string, std::string> ShaderCompiler::InferShaderTypeAndEntry(const std::string& filename) {
    std::string lower_filename = filename;
    std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);
    
    // 根据文件名模式推断shader类型和入口点
    if (lower_filename.find("_vs.hlsl") != std::string::npos) {
        return {"vs_5_0", "VS"};
    } else if (lower_filename.find("_ps.hlsl") != std::string::npos) {
        return {"ps_5_0", "PS"};
    } else if (lower_filename.find("_gs.hlsl") != std::string::npos) {
        return {"gs_5_0", "GS"};
    } else if (lower_filename.find("_hs.hlsl") != std::string::npos) {
        return {"hs_5_0", "HS"};
    } else if (lower_filename.find("_ds.hlsl") != std::string::npos) {
        return {"ds_5_0", "DS"};
    } else if (lower_filename.find("_cs.hlsl") != std::string::npos) {
        return {"cs_5_0", "CS"};
    }
    
    // 默认假设是pixel shader
    return {"ps_5_0", "PS"};
}

CompilationResult ShaderCompiler::CompileShader(const std::string& filepath, const std::string& entry_point, const std::string& target) {
    CompilationResult result;
    result.filename = FileUtils::GetFilename(filepath);
    result.success = false;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        LOG_DEBUG("Starting compilation: " + filepath);
        
        if (!FileUtils::FileExists(filepath)) {
            result.error_message = "File does not exist: " + filepath;
            LOG_ERROR(result.error_message);
            return result;
        }
        
        // 推断shader类型和入口点
        auto [inferred_target, inferred_entry] = InferShaderTypeAndEntry(filepath);
        
        std::string actual_target = target.empty() ? inferred_target : target;
        std::string actual_entry = entry_point.empty() ? inferred_entry : entry_point;
        
        result.shader_model = actual_target;
        result.entry_point = actual_entry;
        
        LOG_DEBUG("Using target: " + actual_target + ", entry point: " + actual_entry);
        
#ifdef _WIN32
        result = CompileHLSLShader(filepath, actual_entry, actual_target);
#else
        result.error_message = "HLSL compilation not supported on current platform";
        LOG_ERROR(result.error_message);
#endif
        
    } catch (const std::exception& e) {
        result.error_message = "Compilation exception: " + std::string(e.what());
        LOG_ERROR("Compilation exception [" + filepath + "]: " + result.error_message);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.compilation_time_ms = duration.count() / 1000.0;
    
    if (result.success) {
        LOG_INFO("Compilation successful: " + result.filename + " (" + std::to_string(result.compilation_time_ms) + "ms)");
        Logger::Instance().LogCompilationSuccess(result.filename, result.compilation_time_ms);
    } else {
        LOG_ERROR("Compilation failed: " + result.filename + " - " + result.error_message);
        Logger::Instance().LogCompilationError(result.filename, result.error_message);
    }
    
    return result;
}

#ifdef _WIN32
CompilationResult ShaderCompiler::CompileHLSLShader(const std::string& filepath, const std::string& entry_point, const std::string& target) {
    CompilationResult result;
    result.filename = FileUtils::GetFilename(filepath);
    result.success = false;
    result.shader_model = target;
    result.entry_point = entry_point;
    
    // 读取shader源代码
    std::string source_code = FileUtils::ReadFileContent(filepath);
    if (source_code.empty()) {
        result.error_message = "Unable to read file content";
        return result;
    }
    
    // 设置编译标志
    UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
    
#ifdef _DEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
    
    // 设置包含目录
    ID3DInclude* include_handler = nullptr;
    if (!m_include_directory.empty()) {
        // 使用默认的include handler，它会搜索文件系统
        include_handler = D3D_COMPILE_STANDARD_FILE_INCLUDE;
    }
    
    ComPtr<ID3DBlob> shader_blob;
    ComPtr<ID3DBlob> error_blob;
    
    // 编译shader
    HRESULT hr = D3DCompile(
        source_code.c_str(),           // 源代码
        source_code.length(),          // 源代码长度
        filepath.c_str(),              // 文件名（用于错误报告）
        nullptr,                       // 宏定义
        include_handler,               // include handler
        entry_point.c_str(),           // 入口点函数名
        target.c_str(),                // 目标profile
        compile_flags,                 // 编译标志
        0,                            // 效果标志
        &shader_blob,                 // 编译结果
        &error_blob                   // 错误信息
    );
    
    if (SUCCEEDED(hr) && shader_blob) {
        result.success = true;
        result.bytecode_size = std::to_string(shader_blob->GetBufferSize()) + " bytes";
        
        LOG_DEBUG("Compilation successful, bytecode size: " + result.bytecode_size);
        
    } else {
        // 编译失败，提取错误信息
        if (error_blob) {
            const char* error_str = static_cast<const char*>(error_blob->GetBufferPointer());
            result.error_message = std::string(error_str, error_blob->GetBufferSize());
            
            // 清理错误信息中的路径
            size_t pos = result.error_message.find(filepath);
            if (pos != std::string::npos) {
                result.error_message.replace(pos, filepath.length(), FileUtils::GetFilename(filepath));
            }
        } else {
            // 没有详细错误信息，使用HRESULT
            std::ostringstream oss;
            oss << "D3DCompile failed, HRESULT: 0x" << std::hex << hr;
            result.error_message = oss.str();
        }
    }
    
    return result;
}
#endif

std::vector<CompilationResult> ShaderCompiler::CompileAllShaders(const std::string& directory) {
    std::vector<CompilationResult> results;
    
    LOG_INFO("Scanning directory for .hlsl files: " + directory);
    
    // 添加调试信息
    LOG_DEBUG("Directory exists check: " + std::string(FileUtils::DirectoryExists(directory) ? "true" : "false"));
    LOG_DEBUG("Normalized directory path: " + FileUtils::NormalizePath(directory));
    
    auto hlsl_files = FileUtils::GetFilesWithExtension(directory, ".hlsl");
    
    LOG_DEBUG("Found " + std::to_string(hlsl_files.size()) + " files with .hlsl extension");
    
    if (hlsl_files.empty()) {
        LOG_WARNING("No .hlsl files found in directory: " + directory);
        std::cout << "Warning: No .hlsl files found in directory: " << directory << std::endl;
        return results;
    }
    
    LOG_INFO("Found " + std::to_string(hlsl_files.size()) + " .hlsl files");
    std::cout << "Found " << hlsl_files.size() << " .hlsl files:" << std::endl;
    
    for (const auto& file : hlsl_files) {
        std::cout << "  " << FileUtils::GetFilename(file) << std::endl;
    }
    std::cout << std::endl;
    
    // 编译每个文件
    for (size_t i = 0; i < hlsl_files.size(); ++i) {
        const auto& file = hlsl_files[i];
        
        std::cout << "[" << (i + 1) << "/" << hlsl_files.size() << "] Compiling: " 
                  << FileUtils::GetFilename(file) << "..." << std::flush;
        
        auto result = CompileShader(file);
        results.push_back(result);
        
        if (result.success) {
            std::cout << " yes (" << std::fixed << std::setprecision(2) 
                      << result.compilation_time_ms << "ms)" << std::endl;
        } else {
            std::cout << " x" << std::endl;
        }
    }
    
    return results;
}

std::vector<CompilationResult> ShaderCompiler::CompileShaderList(const std::vector<std::string>& filepaths) {
    std::vector<CompilationResult> results;
    
    for (size_t i = 0; i < filepaths.size(); ++i) {
        const auto& filepath = filepaths[i];
        
        std::cout << "[" << (i + 1) << "/" << filepaths.size() << "] Compiling: " 
                  << FileUtils::GetFilename(filepath) << "..." << std::flush;
        
        auto result = CompileShader(filepath);
        results.push_back(result);
        
        if (result.success) {
            std::cout << " yes (" << std::fixed << std::setprecision(2) 
                      << result.compilation_time_ms << "ms)" << std::endl;
        } else {
            std::cout << " x" << std::endl;
        }
    }
    
    return results;
}

void ShaderCompiler::PrintCompilationSummary(const std::vector<CompilationResult>& results) {
    if (results.empty()) {
        std::cout << "No compilation results" << std::endl;
        return;
    }
    
    // 成功的编译
    std::cout << "Successfully compiled files:" << std::endl;
    bool has_success = false;
    for (const auto& result : results) {
        if (result.success) {
            has_success = true;
            std::cout << "  yes " << std::setw(30) << std::left << result.filename
                      << " [" << result.shader_model << "] "
                      << "(" << std::fixed << std::setprecision(2) << result.compilation_time_ms << "ms, "
                      << result.bytecode_size << ")" << std::endl;
        }
    }
    if (!has_success) {
        std::cout << "  (None)" << std::endl;
    }
    
    std::cout << std::endl;
    
    // 失败的编译
    std::cout << "Failed compilation files:" << std::endl;
    bool has_failure = false;
    for (const auto& result : results) {
        if (!result.success) {
            has_failure = true;
            std::cout << "  x " << std::setw(30) << std::left << result.filename
                      << " [" << result.shader_model << "]" << std::endl;
            
            // 显示错误信息的前几行
            std::istringstream iss(result.error_message);
            std::string line;
            int line_count = 0;
            while (std::getline(iss, line) && line_count < 3) {
                if (!line.empty()) {
                    std::cout << "    " << line << std::endl;
                    line_count++;
                }
            }
            if (line_count == 3) {
                std::cout << "    ... (More error details available in log files)" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    if (!has_failure) {
        std::cout << "  (None)" << std::endl;
    }
}

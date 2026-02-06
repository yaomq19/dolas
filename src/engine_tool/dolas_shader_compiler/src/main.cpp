#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cctype>
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

void InitializeLogger() {
    // Initialize logging system
    try {
        auto current_time = std::chrono::system_clock::now();
        
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
    }
}

std::string GetShaderDir() {
    // Set shader directory
#ifdef SHADER_CONTENT_DIR
    std::string raw_shader_dir = SHADER_CONTENT_DIR;
#else
    std::string raw_shader_dir = "content/shader";
#endif

    // Normalize path
    std::string shader_dir = FileUtils::NormalizePath(raw_shader_dir);

    LOG_DEBUG("Raw shader directory: " + raw_shader_dir);
    LOG_DEBUG("Normalized shader directory: " + shader_dir);

    if (!FileUtils::DirectoryExists(shader_dir)) {
        LOG_ERROR("Shader directory does not exist: " + shader_dir);
        std::cerr << "Error: Shader directory does not exist: " << shader_dir << std::endl;
        return "";
    }

    LOG_INFO("Shader search directory: " + shader_dir);
    std::cout << "Shader search directory: " << shader_dir << std::endl;

    return shader_dir;
}

int main(int argc, char* argv[]) {
    PrintHeader();
    
    // Check help arguments
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }
    
    std::string shader_dir = GetShaderDir();
    if (shader_dir.empty()) {
        return 1;
    }
    
    // Initialize compiler
    ShaderCompiler compiler;
    compiler.SetIncludeDirectory(shader_dir);

    int command_index = 0;

    while (true) {
        ++command_index;

        std::cout << std::endl;
        std::cout << "=====================================================" << std::endl;
        std::cout << "                    Command " << command_index << std::endl;
        std::cout << "=====================================================" << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;
        std::cout << "Enter shader file name to compile (relative to directory: " << shader_dir << ")" << std::endl;
        std::cout << "  - Type 'all'   : Compile all .hlsl files in this directory" << std::endl;
        std::cout << "  - Type 'list'  : List all *_vs.hlsl and *_ps.hlsl files (absolute paths)" << std::endl;
        std::cout << "  - Type 'quit'  : Exit program" << std::endl;
        std::cout << "Input: " << std::flush;

        std::string input;
        if (!std::getline(std::cin, input)) {
            // EOF or input error: exit loop
            break;
        }

        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        };
        trim(input);

        if (input.empty()) {
            continue;
        }

        if (input == "quit" || input == "exit") {
            std::cout << "Exiting Shader Compiler." << std::endl;
            break;
        }

        if (input == "list") {
            std::cout << std::endl;
            std::cout << "Listing all *_vs.hlsl and *_ps.hlsl files under: " << shader_dir << std::endl;

            // 先递归拿到所有 .hlsl，再用文件名后缀过滤 *_vs.hlsl / *_ps.hlsl
            auto all_hlsl_files = FileUtils::GetFilesWithExtension(shader_dir, ".hlsl");
            std::vector<std::string> vs_files;
            std::vector<std::string> ps_files;

            for (const auto& path : all_hlsl_files) {
                std::string filename = FileUtils::GetFilename(path);
                if (filename.size() >= 8 &&
                    filename.compare(filename.size() - 8, 8, "_vs.hlsl") == 0) {
                    vs_files.push_back(path);
                } else if (filename.size() >= 8 &&
                           filename.compare(filename.size() - 8, 8, "_ps.hlsl") == 0) {
                    ps_files.push_back(path);
                }
            }

            auto print_files = [&](const std::vector<std::string>& files, const char* label) {
                std::cout << std::endl;
                std::cout << "  [" << label << "]" << std::endl;
                if (files.empty()) {
                    std::cout << "    (None)" << std::endl;
                    return;
                }
                for (const auto& path : files) {
                    // FileUtils::GetFilesWithExtension 已经返回规范化的绝对路径
                    std::cout << "    " << path << std::endl;
                }
            };

            print_files(vs_files, "Vertex Shaders (*_vs.hlsl)");
            print_files(ps_files, "Pixel Shaders (*_ps.hlsl)");

            std::cout << std::endl;
            std::cout << "Total vertex shaders: " << vs_files.size() << std::endl;
            std::cout << "Total pixel shaders : " << ps_files.size() << std::endl;
            std::cout << std::endl;

            // Skip compilation part for this command
            continue;
        }

        std::vector<CompilationResult> results;
        auto start_time = std::chrono::high_resolution_clock::now();

        try {
            if (input == "all") {
                // Compile all shader files
                LOG_INFO("Starting to compile all shader files (interactive)...");
                std::cout << "Scanning and compiling all .hlsl files..." << std::endl;

                results = compiler.CompileAllShaders(shader_dir);
            } else {
                // Compile a single specified file (or relative path)
                std::string filepath = input;

                // If it is a relative path, prepend shader directory
                if (filepath.find('/') == std::string::npos && filepath.find('\\') == std::string::npos) {
                    filepath = FileUtils::JoinPath(shader_dir, filepath);
                }

                if (!FileUtils::FileExists(filepath)) {
                    LOG_WARNING("File does not exist: " + filepath);
                    std::cout << "Warning: File does not exist: " << filepath << std::endl;
                    continue;
                }

                LOG_INFO("Starting to compile single file: " + filepath);
                std::cout << "Compiling file: " << filepath << std::endl;

                std::vector<std::string> files_to_compile{ filepath };
                results = compiler.CompileShaderList(files_to_compile);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception occurred during compilation: " + std::string(e.what()));
            std::cerr << "Error: Exception occurred during compilation: " << e.what() << std::endl;
            continue;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Print compilation summary
        std::cout << std::endl;
        std::cout << "=====================================================" << std::endl;
        std::cout << "               Compilation Summary" << std::endl;
        std::cout << "=====================================================" << std::endl;

        compiler.PrintCompilationSummary(results);

        // Statistics
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

        // If there are failures, print detailed error information
        if (failed > 0) {
            std::cout << std::endl;
            std::cout << "================ Failed Details ================" << std::endl;
            for (const auto& result : results) {
                if (!result.success) {
                    std::cout << "File: " << result.filename << std::endl;
                    std::cout << "Shader Model: " << result.shader_model << "  Entry: " << result.entry_point << std::endl;
                    std::cout << "Error Message:" << std::endl;
                    std::cout << result.error_message << std::endl;
                    std::cout << "-----------------------------------------------" << std::endl;
                }
            }
            std::cout << "Detailed error logs have also been written to logs/compilation_errors.log" << std::endl;
        } else {
            std::cout << "All shader files compiled successfully!" << std::endl;
        }

        // Flush logs
        Logger::Instance().Flush();

        std::cout << std::endl;
    }

    return 0;
}

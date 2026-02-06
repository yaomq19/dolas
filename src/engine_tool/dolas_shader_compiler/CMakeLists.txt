cmake_minimum_required(VERSION 3.10)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(ShaderCompiler
    src/main.cpp
    src/shader_compiler.cpp
    src/shader_compiler.h
    src/file_utils.cpp
    src/file_utils.h
    src/logger.cpp
    src/logger.h
)

# 包含目录
target_include_directories(ShaderCompiler PRIVATE
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/../content/shader
)

# Windows特定设置
if(WIN32)
    # 链接D3DCompiler库
    target_link_libraries(ShaderCompiler d3dcompiler)
    
    # 设置为控制台应用程序
    set_target_properties(ShaderCompiler PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(ShaderCompiler PRIVATE /W4)
else()
    target_compile_options(ShaderCompiler PRIVATE -Wall -Wextra -pedantic)
endif()

# 定义宏
target_compile_definitions(ShaderCompiler PRIVATE
    SHADER_CONTENT_DIR="${CMAKE_SOURCE_DIR}/content/shader"
    LOG_OUTPUT_DIR="${CMAKE_BINARY_DIR}/logs"
)

set_target_properties(ShaderCompiler PROPERTIES FOLDER "EngineTool")

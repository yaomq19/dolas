cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Dolas LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定子系统为Windows
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# 收集源文件
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# 设置包含目录
include_directories("source")

# 生成可执行文件
# 使用项目名作为可执行文件名
add_executable(${PROJECT_NAME} ${SOURCES})

# Link DirectX 11 library
target_link_libraries(${PROJECT_NAME} d3d11)
# Link DirectX Compiler library
target_link_libraries(${PROJECT_NAME} d3dcompiler)

# Set the character set to Unicode
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")
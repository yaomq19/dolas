cmake_minimum_required(VERSION 3.10)

# Set the project name
project(dolas LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定子系统为Windows
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# 收集源文件
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# 生成可执行文件
add_executable(dolas ${SOURCES})

# Link DirectX 11 library
target_link_libraries(dolas d3d11)
# Link DirectX Compiler library
target_link_libraries(dolas d3dcompiler)

# Set the character set to Unicode
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")
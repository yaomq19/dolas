# 指定CMake的最低版本要求为3.10，确保使用的CMake功能可用
cmake_minimum_required(VERSION 3.10)

# 定义项目名称为Dolas，版本为0.1.0，使用C++语言
project(Dolas VERSION 0.1.0 LANGUAGES CXX)

# ============ C++标准设置 ============
# 设置项目使用C++17标准
set(CMAKE_CXX_STANDARD 17)
# 强制要求C++17，如果编译器不支持则报错
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============ Windows平台特定设置 ============
# 检查当前平台是否为Windows
if(WIN32)
    # DirectX在Windows SDK中已包含，不需要find_package
    # 只需确保包含正确的头文件路径
    # 添加Windows SDK的包含目录，使用环境变量获取SDK路径
    include_directories("$ENV{WindowsSdkDir}Include/$ENV{WindowsSDKVersion}/um")
    # 添加Windows SDK的库目录，针对x64架构
    link_directories("$ENV{WindowsSdkDir}Lib/$ENV{WindowsSDKVersion}/um/x64")
endif()

# ============ 输出目录配置 ============
# 设置可执行文件的输出目录为构建目录下的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 设置动态库的输出目录为构建目录下的lib文件夹
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置静态库的输出目录为构建目录下的lib文件夹
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============ 源文件自动发现 ============
# 递归搜索source目录下的所有.cpp文件
file(GLOB_RECURSE SOURCES "source/*.cpp")
# 递归搜索source目录下的所有.h文件
file(GLOB_RECURSE HEADERS "source/*.h")

# ============ IDE文件组织结构设置 ============
# 遍历所有源文件和头文件
foreach(source IN LISTS SOURCES HEADERS)
    # 获取文件所在的目录路径
    get_filename_component(source_path "${source}" PATH)
    # 移除项目根目录路径，得到相对路径
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" relative_source_path "${source_path}")
    # 将Unix风格的路径分隔符替换为Windows风格，适配Visual Studio
    string(REPLACE "/" "\\" group_path "${relative_source_path}")
    # 在IDE中创建对应的文件夹结构
    source_group("${group_path}" FILES "${source}")
endforeach()

# ============ 包含目录设置 ============
# 添加项目的包含目录
include_directories(
    # 添加source目录，使得可以使用#include "header.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    # 添加第三方库目录
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# ============ 可执行文件创建 ============
# 创建名为${PROJECT_NAME}(即Dolas)的可执行文件，包含所有源文件和头文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ============ 库链接配置 ============
# 如果是Windows平台，链接DirectX相关库
if(WIN32)
    # 为目标链接必要的DirectX库
    target_link_libraries(${PROJECT_NAME} PRIVATE
        # Direct3D 11核心库
        d3d11.lib
        # DirectX图形基础设施库
        dxgi.lib
        # DirectX着色器编译器库
        d3dcompiler.lib
    )
endif()

# ============ 第三方库集成 ============
# 检查third_party目录下是否存在CMakeLists.txt文件
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/CMakeLists.txt")
    # 如果存在，则将其作为子目录添加到构建系统中
    add_subdirectory(third_party)
endif()

# ============ 调试信息输出 ============
# 打印找到的源文件列表（用于调试和确认）
message(STATUS "Found ${CMAKE_PROJECT_NAME} source files:")
# 遍历并输出每个源文件路径
foreach(source ${SOURCES})
    message(STATUS "  ${source}")
endforeach()

# 打印找到的头文件列表（用于调试和确认）
message(STATUS "Found ${CMAKE_PROJECT_NAME} header files:")
# 遍历并输出每个头文件路径
foreach(header ${HEADERS})
    message(STATUS "  ${header}")
endforeach()

# ============ 全局宏定义 ============
# 定义项目根目录宏，供C++代码使用
add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# 定义项目名称宏
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
# 定义项目版本宏
add_definitions(-DPROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
# 定义项目版本宏
add_definitions(-DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
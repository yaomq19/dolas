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

# ============ 全局变量和宏定义 ============
# 定义项目根目录CMake变量，供CMakeLists.txt使用
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# 定义项目根目录宏，供C++代码使用
add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# 定义项目名称宏
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
# 定义项目版本宏
add_definitions(-DPROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
# 定义项目版本宏
add_definitions(-DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# ============ 子目录包含 ============
# 添加第三方库子目录
add_subdirectory(third_party)

# 添加源代码子目录
add_subdirectory(source)
# 指定CMake的最低版本要求为3.15
cmake_minimum_required(VERSION 3.15)

# 定义项目名称为Dolas，版本为0.1.0，使用C++语言
project(Dolas VERSION 0.1.0 LANGUAGES CXX)

# 启用 IDE 文件夹功能（用于 Visual Studio 的解决方案资源管理器）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 是否启用 Vulkan 支持（默认关闭，对应 README 中"Vulkan SDK（可选）"）
option(ENABLE_VULKAN "Enable Vulkan renderer / SDK support" OFF)

# ============ C++标准设置 ============
# 设置项目使用C++20标准
set(CMAKE_CXX_STANDARD 20)
# 强制要求C++20，如果编译器不支持则报错
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

    # 查找 Vulkan SDK（可选）
    if(ENABLE_VULKAN)
        find_package(Vulkan REQUIRED)
        if(Vulkan_FOUND)
            message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
            include_directories(${Vulkan_INCLUDE_DIRS})
        else()
            message(FATAL_ERROR "Could not find Vulkan SDK")
        endif()
    else()
        message(STATUS "Vulkan support is disabled (ENABLE_VULKAN=OFF).")
    endif()

    # 设置为 UTF-8 编码
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

# ============ 输出目录配置 ============
# 设置可执行文件的输出目录为构建目录下的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 设置动态库的输出目录为构建目录下的lib文件夹
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置静态库的输出目录为构建目录下的lib文件夹
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
 
# ============ 子目录包含 ============
# 添加第三方库子目录（优先构建）
add_subdirectory(third_party)

# 添加源代码子目录
add_subdirectory(engine)

# 添加工具子目录
add_subdirectory(tool)

# 添加 precompile（RSD -> C++ 头文件生成）
add_subdirectory(precompile)

# 让 Dolas 可执行目标依赖 Precompile（确保先生成 source/rsd/*.h）
if(TARGET Dolas AND TARGET Precompile)
    add_dependencies(Dolas Precompile)
endif()

# ============ IDE 项目文件夹组织 ============
# 将 CMake 预定义的工具项目归类到 CMakeTools 文件夹
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTools")

# 将 Dolas 引擎相关项目归类到 DolasEngine 文件夹
if(TARGET Dolas)
    set_target_properties(Dolas PROPERTIES FOLDER "DolasEngine")
endif()
if(TARGET ShaderCompiler)
    set_target_properties(ShaderCompiler PROPERTIES FOLDER "DolasEngine")
endif()
if(TARGET Precompile)
    set_target_properties(Precompile PROPERTIES FOLDER "DolasEngine")
endif()
if(TARGET ModelImporter)
    set_target_properties(ModelImporter PROPERTIES FOLDER "DolasEngine")
endif()

# 注意：
# - DolasAssetEditor 的 FOLDER 在 asset_editor/CMakeLists.txt 中设置
# - ThirdParty 文件夹已经在 third_party/CMakeLists.txt 中设置
# - 各第三方库已经被组织到 "ThirdParty" 文件夹中

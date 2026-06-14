# 指定CMake的最低版本要求为3.15
cmake_minimum_required(VERSION 3.15)

# 定义项目名称为Dolas，版本为0.1.0，使用C++语言
project(Dolas VERSION 0.1.0 LANGUAGES CXX)

# 启用 IDE 文件夹功能（用于 Visual Studio 的解决方案资源管理器）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 渲染后端选择。Windows 默认保留现有 D3D 路径，macOS 默认走 Vulkan/MoltenVK。
if(WIN32)
    set(DOLAS_DEFAULT_RENDER_BACKEND "D3D12")
else()
    set(DOLAS_DEFAULT_RENDER_BACKEND "Vulkan")
endif()

set(DOLAS_RENDER_BACKEND "${DOLAS_DEFAULT_RENDER_BACKEND}" CACHE STRING "Render backend: D3D12 or Vulkan")
set_property(CACHE DOLAS_RENDER_BACKEND PROPERTY STRINGS D3D12 Vulkan)

# 是否启用 Vulkan 支持（保留旧选项，后端选择为 Vulkan 时自动启用）
option(ENABLE_VULKAN "Enable Vulkan renderer / SDK support" OFF)
if(DOLAS_RENDER_BACKEND STREQUAL "Vulkan")
    set(ENABLE_VULKAN ON CACHE BOOL "Enable Vulkan renderer / SDK support" FORCE)
endif()

if(NOT DOLAS_RENDER_BACKEND STREQUAL "D3D12" AND NOT DOLAS_RENDER_BACKEND STREQUAL "Vulkan")
    message(FATAL_ERROR "Unsupported DOLAS_RENDER_BACKEND='${DOLAS_RENDER_BACKEND}'. Expected D3D12 or Vulkan.")
endif()

function(dolas_enable_utf8 target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /utf-8)
    endif()
endfunction()

# ============ Windows平台特定设置 ============
# 检查当前平台是否为Windows
if(WIN32)
    # DirectX在Windows SDK中已包含，不需要find_package
    # 只需确保包含正确的头文件路径
    # 添加Windows SDK的包含目录，使用环境变量获取SDK路径
    include_directories("$ENV{WindowsSdkDir}Include/$ENV{WindowsSDKVersion}/um")
    # 添加Windows SDK的库目录，针对x64架构
    link_directories("$ENV{WindowsSdkDir}Lib/$ENV{WindowsSDKVersion}/um/x64")

    # 设置为 UTF-8 编码
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

if(ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR "Could not find Vulkan SDK. Install LunarG Vulkan SDK or Homebrew Vulkan packages, then set VULKAN_SDK if needed.")
    endif()
    message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

if(APPLE AND DOLAS_RENDER_BACKEND STREQUAL "Vulkan")
    find_package(glfw3 CONFIG QUIET)
    if(TARGET glfw)
        set(DOLAS_GLFW_TARGET glfw CACHE INTERNAL "GLFW target")
    elseif(TARGET glfw3::glfw)
        set(DOLAS_GLFW_TARGET glfw3::glfw CACHE INTERNAL "GLFW target")
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLFW3 QUIET IMPORTED_TARGET glfw3)
        endif()
        if(TARGET PkgConfig::GLFW3)
            set(DOLAS_GLFW_TARGET PkgConfig::GLFW3 CACHE INTERNAL "GLFW target")
        else()
            message(FATAL_ERROR "Could not find GLFW3 for macOS Vulkan. Install it with 'brew install glfw' or provide glfw3_DIR/PKG_CONFIG_PATH.")
        endif()
    endif()

    find_program(DOLAS_DXC_EXECUTABLE NAMES dxc HINTS "$ENV{VULKAN_SDK}/bin")
    find_program(DOLAS_GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator HINTS "$ENV{VULKAN_SDK}/bin")
    if(NOT DOLAS_DXC_EXECUTABLE AND NOT DOLAS_GLSLANG_VALIDATOR_EXECUTABLE)
        message(FATAL_ERROR "Could not find a Vulkan shader compiler. Install dxc or glslangValidator, for example from the Vulkan SDK, and ensure it is on PATH.")
    endif()
endif()

# ============ 输出目录配置 ============
# 设置可执行文件的输出目录为构建目录下的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 设置动态库的输出目录为构建目录下的lib文件夹
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置静态库的输出目录为构建目录下的lib文件夹
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
 
# ============ 测试支持 ============
# 启用 CTest 测试框架
enable_testing()

# ============ 子目录包含 ============
# 添加第三方库子目录（优先构建）
add_subdirectory(third_party)

# 添加源代码子目录
add_subdirectory(src)

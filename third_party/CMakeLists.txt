# Third-party libraries
# This directory contains external dependencies as git submodules

# ============ ImGui ============
# ImGui (docking branch with multi-viewport support)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# ImGui 核心文件
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
)

# ImGui 后端文件（Win32 + DX11）
set(IMGUI_BACKEND_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

set(IMGUI_BACKEND_HEADERS
    ${IMGUI_DIR}/backends/imgui_impl_win32.h
    ${IMGUI_DIR}/backends/imgui_impl_dx11.h
)

# 创建 ImGui 静态库
add_library(imgui STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_HEADERS}
    ${IMGUI_BACKEND_SOURCES}
    ${IMGUI_BACKEND_HEADERS}
)

# 设置 ImGui 包含目录
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ImGui 需要的系统库（Windows）
if(WIN32)
    target_link_libraries(imgui PUBLIC
        d3d11
        dxgi
        d3dcompiler
        dwmapi
    )
endif()

# 设置 ImGui 的编译选项
target_compile_features(imgui PUBLIC cxx_std_11)

# 如果是 MSVC，设置为 UTF-8 编码
if(MSVC)
    target_compile_options(imgui PRIVATE /utf-8)
endif()

# 将 ImGui 组织到 IDE 的 "ThirdParty" 文件夹中
set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")

# ============ Assimp ============
# Assimp - Open Asset Import Library
set(ASSIMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assimp)

# 配置 Assimp 构建选项
option(ASSIMP_BUILD_TESTS "Build Assimp tests" OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS "Build Assimp tools" OFF)
option(ASSIMP_BUILD_SAMPLES "Build Assimp samples" OFF)
option(ASSIMP_BUILD_ZLIB "Build Assimp's own zlib" ON)
option(ASSIMP_INSTALL "Install Assimp" OFF)
option(ASSIMP_NO_EXPORT "Disable Assimp exporters" OFF)
option(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT "Build all exporters by default" OFF)
option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "Build all importers by default" ON)

# 禁用不必要的警告
if(MSVC)
    add_compile_options(/wd4251 /wd4244 /wd4267)
endif()

# 添加 Assimp 子目录
add_subdirectory(${ASSIMP_DIR})

# 将 Assimp 及其相关目标组织到 IDE 的 "ThirdParty" 文件夹中
set_target_properties(assimp PROPERTIES FOLDER "ThirdParty")

# 如果构建了 zlib，也放到 ThirdParty 文件夹
if(TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "ThirdParty")
endif()

if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "ThirdParty")
endif()

# ============ TinyXML2 ============
# TinyXML2 - Simple XML parser
set(TINYXML2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)

# 配置 TinyXML2 构建选项
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(tinyxml2_INSTALL_PKGCONFIG OFF CACHE BOOL "" FORCE)

# 添加 TinyXML2 子目录
add_subdirectory(${TINYXML2_DIR} EXCLUDE_FROM_ALL)

# 将 TinyXML2 组织到 IDE 的 "ThirdParty" 文件夹中
if(TARGET tinyxml2)
    set_target_properties(tinyxml2 PROPERTIES FOLDER "ThirdParty")
endif()
if(TARGET tinyxml2_static)
    set_target_properties(tinyxml2_static PROPERTIES FOLDER "ThirdParty")
endif()
if(TARGET xmltest)
    set_target_properties(xmltest PROPERTIES FOLDER "ThirdParty")
endif()

# ============ spdlog ============
# spdlog - Fast C++ logging library
set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spdlog)

# 配置 spdlog 构建选项
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)

# 添加 spdlog 子目录
add_subdirectory(${SPDLOG_DIR})

# 将 spdlog 组织到 IDE 的 "ThirdParty" 文件夹中
if(TARGET spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER "ThirdParty")
endif()
if(TARGET spdlog_headers_for_ide)
    set_target_properties(spdlog_headers_for_ide PROPERTIES FOLDER "ThirdParty")
endif()

# ============ DirectXTex ============
# DirectXTex - Texture processing library
set(DIRECTXTEX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DirectXTex)

# 配置 DirectXTex 构建选项
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(BUILD_DX11 ON CACHE BOOL "" FORCE)
set(BUILD_DX12 OFF CACHE BOOL "" FORCE)

# 添加 DirectXTex 子目录
add_subdirectory(${DIRECTXTEX_DIR})

# 将 DirectXTex 组织到 IDE 的 "ThirdParty" 文件夹中
if(TARGET DirectXTex)
    set_target_properties(DirectXTex PROPERTIES FOLDER "ThirdParty")
endif()

# ============ Tracy ============
# Tracy - Frame profiler
set(TRACY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tracy)

# 配置 Tracy 构建选项
set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
set(TRACY_NO_CRASH_HANDLER OFF CACHE BOOL "" FORCE)
set(TRACY_NO_SAMPLING OFF CACHE BOOL "" FORCE)

# 添加 Tracy 子目录
add_subdirectory(${TRACY_DIR})

# 将 Tracy 组织到 IDE 的 "ThirdParty" 文件夹中
if(TARGET TracyClient)
    set_target_properties(TracyClient PROPERTIES FOLDER "ThirdParty")
endif()

# ============ Catch2 ============
# Catch2 - Modern C++ test framework
set(CATCH2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Catch2)

# 配置 Catch2 构建选项
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_EXTRA_TESTS OFF CACHE BOOL "" FORCE)
set(CATCH_ENABLE_COVERAGE OFF CACHE BOOL "" FORCE)
set(CATCH_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

# 添加 Catch2 子目录
add_subdirectory(${CATCH2_DIR})

# 将 Catch2 组织到 IDE 的 "ThirdParty" 文件夹中
if(TARGET Catch2)
    set_target_properties(Catch2 PROPERTIES FOLDER "ThirdParty")
endif()
if(TARGET Catch2WithMain)
    set_target_properties(Catch2WithMain PROPERTIES FOLDER "ThirdParty")
endif()

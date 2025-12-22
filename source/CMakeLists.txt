# ============ 源文件自动发现 ============
# 递归搜索当前目录下的所有.cpp文件
file(GLOB_RECURSE SOURCES "*.cpp")
# 递归搜索当前目录下的所有.h文件
file(GLOB_RECURSE HEADERS "*.h")

# ============ 应用程序宏定义 ============
# 定义项目根目录宏，供C++代码使用（使用CMake内建变量CMAKE_SOURCE_DIR）
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}/")
# 定义运行时输出目录宏
add_definitions(-DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")

# ============ IDE文件组织结构设置 ============
# 遍历所有源文件和头文件
foreach(source IN LISTS SOURCES HEADERS)
    # 获取文件所在的目录路径
    get_filename_component(source_path "${source}" PATH)
    # 移除当前源目录路径，得到相对路径
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" relative_source_path "${source_path}")
    # 将Unix风格的路径分隔符替换为Windows风格，适配Visual Studio
    string(REPLACE "/" "\\" group_path "${relative_source_path}")
    # 在IDE中创建对应的文件夹结构
    source_group("${group_path}" FILES "${source}")
endforeach()

# ============ 包含目录设置 ============
# 添加项目的包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(spdlog CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(directxtex CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# ============ 可执行文件创建 ============
# 创建名为${PROJECT_NAME}(即Dolas)的可执行文件，包含所有源文件和头文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ============ spdlog 配置 ============
# 配置 spdlog 使用 C++ 标准库的 std::format 而不是外部 fmt 库
target_compile_definitions(${PROJECT_NAME} PRIVATE SPDLOG_USE_STD_FORMAT)

# ============ Tracy 配置 ============
# 启用 Tracy 性能分析（仅在需要时）
target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE)

# ============ 库链接配置 ============
# 使用 spdlog 的 header-only 版本（不需要链接库文件）
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
# XML
target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2::tinyxml2)
# 链接 assimp
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)
# 链接 DirectXTex
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXTex)
# 链接 Tracy
target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
# 链接 ImGui
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

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
        # DirectX GUID 库（提供 IID 和其他标识符）
        dxguid.lib
        # 输入法控制（ImmGetContext / ImmSetOpenStatus 等）
        imm32.lib
    )
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
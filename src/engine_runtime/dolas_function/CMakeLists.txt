add_library(DolasFunction STATIC)

# ============ 源文件自动发现 ============
# 递归搜索当前目录下的所有.cpp文件（排除已在子目录中独立管理的模块）
file(GLOB_RECURSE SOURCES "*.cpp")
# 递归搜索当前目录下的所有.h文件
file(GLOB_RECURSE HEADERS "*.h")

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

target_sources(DolasFunction PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(DolasFunction PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)
target_include_directories(DolasFunction PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private/include
)

target_compile_features(DolasFunction PRIVATE cxx_std_20)
target_compile_options(DolasFunction PRIVATE /utf-8)

target_link_libraries(DolasFunction PRIVATE DolasResource)
target_link_libraries(DolasFunction PRIVATE DolasCore)
target_link_libraries(DolasFunction PRIVATE DolasPlatform)
# ============ spdlog 配置 ============
# 配置 spdlog 使用 C++ 标准库的 std::format 而不是外部 fmt 库
target_compile_definitions(DolasFunction PRIVATE SPDLOG_USE_STD_FORMAT)

# ============ Tracy 配置 ============
# 启用 Tracy 性能分析（仅在需要时）
target_compile_definitions(DolasFunction PRIVATE TRACY_ENABLE)

# 链接 spdlog（来自 third_party 子模块，header-only 版本）
target_link_libraries(DolasFunction PRIVATE spdlog::spdlog_header_only)
# 链接 TinyXML2（来自 third_party 子模块）
target_link_libraries(DolasFunction PRIVATE tinyxml2)
# 链接 Assimp（来自 third_party 子模块）
target_link_libraries(DolasFunction PRIVATE assimp)
# 链接 DirectXTex（来自 third_party 子模块）
target_link_libraries(DolasFunction PRIVATE DirectXTex)
# 链接 Tracy（来自 third_party 子模块）
target_link_libraries(DolasFunction PRIVATE TracyClient)
# 链接 ImGui（来自 third_party 子模块）
target_link_libraries(DolasFunction PRIVATE imgui)

# 如果是Windows平台，链接DirectX相关库
if(WIN32)
    # 为目标链接必要的DirectX库
    target_link_libraries(DolasFunction PRIVATE
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

set_target_properties(DolasFunction PROPERTIES FOLDER "EngineRuntime")
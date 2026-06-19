add_library(DolasPlatform STATIC)

# ============ 源文件自动发现 ============
# 递归搜索当前目录下的所有.cpp文件（排除已在子目录中独立管理的模块）
file(GLOB_RECURSE SOURCES "*.cpp")
# 递归搜索当前目录下的所有.h文件
file(GLOB_RECURSE HEADERS "*.h")

if(NOT WIN32)
    list(FILTER SOURCES EXCLUDE REGEX "dolas_render_hardware_interface\\.cpp$")
    list(FILTER HEADERS EXCLUDE REGEX "dolas_render_hardware_interface\\.h$")
endif()

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

target_sources(DolasPlatform PRIVATE ${SOURCES} ${HEADERS})
target_link_libraries(DolasPlatform PRIVATE DolasCommon)

# 链接 D3D12 库
if(WIN32)
    target_link_libraries(DolasPlatform PRIVATE d3d12.lib dxgi.lib dxguid.lib)
endif()

target_include_directories(DolasPlatform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)
target_include_directories(DolasPlatform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private/include
)

target_compile_features(DolasPlatform PRIVATE cxx_std_20)
dolas_enable_utf8(DolasPlatform)

set_target_properties(DolasPlatform PROPERTIES FOLDER "EngineRuntime")

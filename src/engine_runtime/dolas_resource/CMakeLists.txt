add_library(DolasResource STATIC)

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

target_sources(DolasResource PRIVATE ${SOURCES} ${HEADERS})
target_include_directories(DolasResource PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)
target_include_directories(DolasResource PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private/include
)
target_link_libraries(DolasResource PRIVATE DolasCore)
target_compile_features(DolasResource PRIVATE cxx_std_20)
target_compile_options(DolasResource PRIVATE /utf-8)
set_target_properties(DolasResource PROPERTIES FOLDER "EngineRuntime")
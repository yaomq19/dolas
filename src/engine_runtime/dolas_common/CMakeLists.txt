add_library(DolasCommon STATIC)

# ============ 源文件自动发现 ============
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# ============ IDE文件组织结构设置 ============
foreach(source IN LISTS SOURCES HEADERS)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" relative_source_path "${source_path}")
    string(REPLACE "/" "\\" group_path "${relative_source_path}")
    source_group("${group_path}" FILES "${source}")
endforeach()

target_sources(DolasCommon PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(DolasCommon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
)
target_include_directories(DolasCommon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(DolasCommon PRIVATE spdlog::spdlog_header_only)

target_compile_features(DolasCommon PRIVATE cxx_std_20)
dolas_enable_utf8(DolasCommon)

set_target_properties(DolasCommon PROPERTIES FOLDER "EngineRuntime")

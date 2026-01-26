cmake_minimum_required(VERSION 3.15)
project(DolasEditor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(DolasEditor
    main.cpp
)

# 包含目录：需要 engine 的头文件
target_include_directories(DolasEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/rsd  # 生成的 RSD 头文件
)

target_link_libraries(DolasEditor PRIVATE DolasEngine)


# 编译选项
if(MSVC)
    target_compile_options(DolasEditor PRIVATE /utf-8)
    target_compile_definitions(DolasEditor PRIVATE
        SPDLOG_USE_STD_FORMAT
        TRACY_ENABLE
    )
endif()

# IDE 文件夹组织
set_target_properties(DolasEditor PROPERTIES FOLDER "DolasEngine")
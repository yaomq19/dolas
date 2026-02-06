cmake_minimum_required(VERSION 3.15)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(DolasEditor
    main.cpp
)

target_link_libraries(DolasEditor PRIVATE DolasFunction)
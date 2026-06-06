cmake_minimum_required(VERSION 3.15)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(DolasEditor
    main.cpp
)

target_link_libraries(DolasEditor PRIVATE DolasFunction)

# Windows 资源文件（图标等）
if(WIN32)
    target_sources(DolasEditor PRIVATE ${CMAKE_SOURCE_DIR}/rc/Dolas.rc)
endif()

set_target_properties(DolasEditor PROPERTIES FOLDER "EngineTool")
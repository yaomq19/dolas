cmake_minimum_required(VERSION 3.10)

# Set the project name
project(render_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

set(HEADER_DIR ".")
set(HEADERS "${SOURCE_DIR}/main.h")

set(SOURCE_DIR ".")

set(
    SOURCES_RENDER 
    "${SOURCE_DIR}/render/a.cpp"
    "${SOURCE_DIR}/render/a.h"
)

set(
    SOURCES_RENDER_SCENE
    "${SOURCE_DIR}/render_scene/b.cpp"
    "${SOURCE_DIR}/render_scene/b.h"
)

set(
    SOURCES 
    ${SOURCES_RENDER}
    ${SOURCES_RENDER_SCENE}
)


add_library(render_engine STATIC ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

target_include_directories(render_engine PUBLIC ${HEADER_DIR})
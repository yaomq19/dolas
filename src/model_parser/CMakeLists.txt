cmake_minimum_required(VERSION 3.10)

# Set the project name
project(model_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")

set(HEADER_DIR ".")
set(SOURCE_DIR ".")

set(
    SOURCES_A
    "${SOURCE_DIR}/a/a.cpp"
    "${SOURCE_DIR}/a/a.h"
)

set(
    SOURCES_B
    "${SOURCE_DIR}/b/b.cpp"
    "${SOURCE_DIR}/b/b.h"
)

set(
    SOURCES 
    ${SOURCES_A}
    ${SOURCES_B}
)


add_library(${PROJECT_NAME} STATIC ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_DIR})
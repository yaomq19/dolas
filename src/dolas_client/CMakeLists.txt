cmake_minimum_required(VERSION 3.10)

# Set the project name
project(dolas_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")

add_executable(${PROJECT_NAME} "main.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "../render_engine")

target_link_libraries(${PROJECT_NAME} PUBLIC render_engine)
target_link_libraries(${PROJECT_NAME} PUBLIC model_parser)
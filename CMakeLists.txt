cmake_minimum_required(VERSION 3.10)

# Set the project name
project(dolas)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add subdirectory for third-party GLFW
add_subdirectory(third_party/glfw-3.4)

# Add the source files
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# Add the executable
add_executable(dolas ${SOURCES})

# Link GLFW library
target_link_libraries(dolas glfw)

# Link DirectX 11 library
target_link_libraries(dolas d3d11)
# Link DirectX Compiler library
target_link_libraries(dolas d3dcompiler)
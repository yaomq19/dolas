cmake_minimum_required(VERSION 3.10)

# Set the project name
project(dolas)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# Add the executable
add_executable(dolas ${SOURCES})
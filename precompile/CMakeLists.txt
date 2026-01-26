project(DolasPrecompile)

# Precompile is now a real C++ executable project.
# Building it will:
#   1) build Precompile.exe
#   2) automatically run it to generate source/rsd/*.h from rsd/*.rsd

file(GLOB DOLAS_RSD_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/rsd/*.rsd")
set(DOLAS_RSD_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/engine/rsd")
file(MAKE_DIRECTORY "${DOLAS_RSD_OUTPUT_DIR}")

add_executable(Precompile
    src/main.cpp
    ${DOLAS_RSD_FILES} # show schemas inside the VS project (also helps VS detect changes)
)

# TinyXML2 现在通过 third_party 子模块构建，不再使用 vcpkg
target_link_libraries(Precompile PRIVATE tinyxml2)
target_compile_features(Precompile PRIVATE cxx_std_20)

# Run after build (VS: right click Precompile -> Build will also execute the generator)
add_custom_command(
    TARGET Precompile
    POST_BUILD
    COMMAND "$<TARGET_FILE:Precompile>"
        --rsd-dir "${CMAKE_SOURCE_DIR}/rsd"
        --out-dir "${DOLAS_RSD_OUTPUT_DIR}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Run Precompile.exe to generate source/rsd/*.h"
    VERBATIM
)

# Visual Studio: ensure post-build runs even if the exe output itself did not change.
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(TARGET Precompile PROPERTY VS_GLOBAL_RunPostBuildEvent "Always")
endif()

set_property(TARGET Precompile PROPERTY FOLDER "precompile")



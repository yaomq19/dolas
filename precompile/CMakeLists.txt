project(DolasPrecompile)

# Generate output directory (write to source tree: source/rsd)
set(DOLAS_RSD_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/source/rsd")
file(MAKE_DIRECTORY "${DOLAS_RSD_OUTPUT_DIR}")

# Collect RSD files
file(GLOB DOLAS_RSD_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/rsd/*.rsd")

set(DOLAS_RSD_GENERATED_HEADERS "")
# Generate headers for each RSD file
foreach(rsd_file IN LISTS DOLAS_RSD_FILES)
    # Get the stem of the RSD file
    get_filename_component(rsd_stem "${rsd_file}" NAME_WE)
    # Calculate output path: out_h = source/rsd/camera.h
    set(out_h "${DOLAS_RSD_OUTPUT_DIR}/${rsd_stem}.h")

    add_custom_command(
        OUTPUT "${out_h}"
        COMMAND "${CMAKE_COMMAND}"
            -DINPUT="${rsd_file}"
            -DOUTPUT="${out_h}"
            -P "${CMAKE_CURRENT_LIST_DIR}/cmake/rsd_generate_one.cmake"
        DEPENDS "${rsd_file}" "${CMAKE_CURRENT_LIST_DIR}/cmake/rsd_generate_one.cmake"
        COMMENT "RSD -> H: ${rsd_stem}.rsd -> ${rsd_stem}.h"
        VERBATIM
    )

    list(APPEND DOLAS_RSD_GENERATED_HEADERS "${out_h}")
endforeach()

# Cleanup: remove stale headers in source/rsd that no longer have a matching .rsd
set(DOLAS_RSD_CLEAN_STAMP "${CMAKE_CURRENT_BINARY_DIR}/rsd_cleanup.stamp")
add_custom_command(
    OUTPUT "${DOLAS_RSD_CLEAN_STAMP}"
    COMMAND "${CMAKE_COMMAND}"
        -DRSD_DIR="${CMAKE_SOURCE_DIR}/rsd"
        -DOUTPUT_DIR="${DOLAS_RSD_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/cmake/rsd_cleanup_headers.cmake"
    COMMAND "${CMAKE_COMMAND}" -E touch "${DOLAS_RSD_CLEAN_STAMP}"
    DEPENDS ${DOLAS_RSD_FILES}
            ${DOLAS_RSD_GENERATED_HEADERS}
            "${CMAKE_CURRENT_LIST_DIR}/cmake/rsd_cleanup_headers.cmake"
    COMMENT "RSD cleanup: remove stale generated headers"
    VERBATIM
)

# "Precompile" project: building it will generate all headers
add_custom_target(Precompile DEPENDS ${DOLAS_RSD_GENERATED_HEADERS} "${DOLAS_RSD_CLEAN_STAMP}")

# Display in VS solution
set_property(TARGET Precompile PROPERTY FOLDER "precompile")



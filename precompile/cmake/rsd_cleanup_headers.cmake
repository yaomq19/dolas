# rsd_cleanup_headers.cmake
#
# Removes stale generated headers in OUTPUT_DIR that no longer have a matching .rsd.
#
# Required -D variables:
#   -DRSD_DIR=".../rsd"
#   -DOUTPUT_DIR=".../source/rsd"
#
# Notes:
# - This is intentionally conservative: it only touches "*.h" files directly in OUTPUT_DIR.
# - It will not delete anything outside OUTPUT_DIR.

if(NOT DEFINED RSD_DIR OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "rsd_cleanup_headers.cmake requires -DRSD_DIR=... -DOUTPUT_DIR=...")
endif()

set(_rsd_dir "${RSD_DIR}")
string(REGEX REPLACE "^\"(.*)\"$" "\\1" _rsd_dir "${_rsd_dir}") # Strip quotes if present

set(_out_dir "${OUTPUT_DIR}")
string(REGEX REPLACE "^\"(.*)\"$" "\\1" _out_dir "${_out_dir}") # Strip quotes if present

if(NOT IS_DIRECTORY "${_out_dir}")
    # Nothing to clean
    return()
endif()

# Collect expected stems from *.rsd (non-recursive).
set(_expected_stems "")
if(IS_DIRECTORY "${_rsd_dir}")
    file(GLOB _rsd_files "${_rsd_dir}/*.rsd")
    foreach(rsd IN LISTS _rsd_files)
        get_filename_component(_stem "${rsd}" NAME_WE)
        list(APPEND _expected_stems "${_stem}")
    endforeach()
endif()

# Collect existing headers in the output directory (non-recursive).
file(GLOB _existing_headers "${_out_dir}/*.h")

foreach(h IN LISTS _existing_headers)
    get_filename_component(_h_stem "${h}" NAME_WE)

    # Safety: never delete this, even if it exists.
    if(_h_stem STREQUAL "rsd_field")
        continue()
    endif()

    # If this header stem is NOT in the expected stems list, delete it.
    list(FIND _expected_stems "${_h_stem}" _idx)
    if(_idx EQUAL -1)
        message(STATUS "Precompile cleanup: removing stale header: ${h}")
        file(REMOVE "${h}")
    endif()
endforeach()



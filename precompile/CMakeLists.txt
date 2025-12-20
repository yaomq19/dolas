project(DolasPrecompile)

# 生成输出目录（写入源码树：source/rsd）
set(DOLAS_RSD_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/source/rsd")
file(MAKE_DIRECTORY "${DOLAS_RSD_OUTPUT_DIR}")

# 收集 rsd 文件
file(GLOB DOLAS_RSD_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/rsd/*.rsd")

set(DOLAS_RSD_GENERATED_HEADERS "")
foreach(rsd_file IN LISTS DOLAS_RSD_FILES)
    get_filename_component(rsd_stem "${rsd_file}" NAME_WE)
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

# “Precompile 工程”：构建它就会生成所有头文件
add_custom_target(Precompile DEPENDS ${DOLAS_RSD_GENERATED_HEADERS})

# VS 解决方案里分组显示
set_property(TARGET Precompile PROPERTY FOLDER "precompile")



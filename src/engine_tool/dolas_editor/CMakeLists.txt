cmake_minimum_required(VERSION 3.15)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(DolasEditor)

target_link_libraries(DolasEditor PRIVATE DolasCommon)

if(APPLE AND DOLAS_RENDER_BACKEND STREQUAL "Vulkan")
    enable_language(OBJCXX)
    set(DOLAS_EDITOR_RUNTIME_SHADER_DIR "${CMAKE_BINARY_DIR}/runtime_shaders")
    file(MAKE_DIRECTORY "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}")
    set(DOLAS_EDITOR_DEBUG_DRAW_VS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/debug_draw_vs.spv")
    set(DOLAS_EDITOR_DEBUG_DRAW_PS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/debug_draw_ps.spv")
    set(DOLAS_EDITOR_OPAQUE_VS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/opaque_vs.spv")
    set(DOLAS_EDITOR_OPAQUE_PS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/opaque_ps.spv")
    set(DOLAS_EDITOR_DEFERRED_SHADING_VS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/deferred_shading_vs.spv")
    set(DOLAS_EDITOR_DEFERRED_SHADING_PS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/deferred_shading_ps.spv")
    set(DOLAS_EDITOR_SKY_BOX_VS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/sky_box_vs.spv")
    set(DOLAS_EDITOR_SKY_BOX_PS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/sky_box_ps.spv")
    set(DOLAS_EDITOR_PRESENT_SCENE_PS_SPV "${DOLAS_EDITOR_RUNTIME_SHADER_DIR}/present_scene_ps.spv")

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_DEBUG_DRAW_VS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                -S vert -e VS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_DEBUG_DRAW_VS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_vs.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_vs.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_DEBUG_DRAW_PS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                -S frag -e PS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_DEBUG_DRAW_PS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_ps.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_ps.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/debug_draw/debug_draw_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/dolas_hlsl_support.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_OPAQUE_VS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                -S vert -e VS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_OPAQUE_VS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_vs.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_vs.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_OPAQUE_PS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                --shift-texture-binding frag 16 --shift-sampler-binding frag 32
                -S frag -e PS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_OPAQUE_PS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_ps.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_ps.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/opaque/opaque_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/shade_mode.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/dolas_hlsl_support.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_DEFERRED_SHADING_VS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                -S vert -e VS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_DEFERRED_SHADING_VS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_vs.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_vs.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_DEFERRED_SHADING_PS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                --shift-texture-binding frag 16 --shift-sampler-binding frag 32
                -S frag -e PS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_DEFERRED_SHADING_PS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_macos_ps.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_macos_ps.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/surface_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/light.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_SKY_BOX_VS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                -S vert -e VS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_SKY_BOX_VS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_vs.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_vs.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_SKY_BOX_PS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                --shift-texture-binding frag 16 --shift-sampler-binding frag 32
                -S frag -e PS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_SKY_BOX_PS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_ps.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_ps.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/sky_box/sky_box_common.hlsli"
            "${CMAKE_SOURCE_DIR}/content/shader/global_constants.hlsli"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${DOLAS_EDITOR_PRESENT_SCENE_PS_SPV}"
        COMMAND "${DOLAS_GLSLANG_VALIDATOR_EXECUTABLE}" -V --target-env vulkan1.2 -D --hlsl-offsets --hlsl-iomap
                --shift-texture-binding frag 16 --shift-sampler-binding frag 32
                -S frag -e PS "-I${CMAKE_SOURCE_DIR}/content/shader"
                -o "${DOLAS_EDITOR_PRESENT_SCENE_PS_SPV}"
                "${CMAKE_SOURCE_DIR}/content/shader/present/present_scene_ps.hlsl"
        DEPENDS
            "${CMAKE_SOURCE_DIR}/content/shader/present/present_scene_ps.hlsl"
            "${CMAKE_SOURCE_DIR}/content/shader/deferred_shading/deferred_shading_common.hlsli"
        VERBATIM
    )

    add_custom_target(DolasEditorRuntimeShaders DEPENDS
        "${DOLAS_EDITOR_DEBUG_DRAW_VS_SPV}"
        "${DOLAS_EDITOR_DEBUG_DRAW_PS_SPV}"
        "${DOLAS_EDITOR_OPAQUE_VS_SPV}"
        "${DOLAS_EDITOR_OPAQUE_PS_SPV}"
        "${DOLAS_EDITOR_DEFERRED_SHADING_VS_SPV}"
        "${DOLAS_EDITOR_DEFERRED_SHADING_PS_SPV}"
        "${DOLAS_EDITOR_SKY_BOX_VS_SPV}"
        "${DOLAS_EDITOR_SKY_BOX_PS_SPV}"
        "${DOLAS_EDITOR_PRESENT_SCENE_PS_SPV}"
    )

    target_sources(DolasEditor PRIVATE main_macos_vulkan.cpp)
    set_source_files_properties(main_macos_vulkan.cpp PROPERTIES LANGUAGE OBJCXX)
    add_dependencies(DolasEditor DolasEditorRuntimeShaders)
    target_link_libraries(DolasEditor PRIVATE
        imgui
        Vulkan::Vulkan
        ${DOLAS_GLFW_TARGET}
        "-framework AppKit"
    )
    target_compile_definitions(DolasEditor PRIVATE
        DOLAS_EDITOR_RUNTIME_SHADER_DIR="${DOLAS_EDITOR_RUNTIME_SHADER_DIR}"
    )
elseif(WIN32)
    target_sources(DolasEditor PRIVATE main.cpp)
    target_link_libraries(DolasEditor PRIVATE DolasFunction)
else()
    message(FATAL_ERROR "DolasEditor currently supports Windows D3D and macOS Vulkan only.")
endif()

# Windows 资源文件（图标等）
if(WIN32)
    target_sources(DolasEditor PRIVATE ${CMAKE_SOURCE_DIR}/rc/Dolas.rc)
endif()

dolas_enable_utf8(DolasEditor)
target_compile_features(DolasEditor PRIVATE cxx_std_20)

set_target_properties(DolasEditor PROPERTIES FOLDER "EngineTool")

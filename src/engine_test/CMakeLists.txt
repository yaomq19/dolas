# ============ Catch2 + CTest 测试项目 ============
add_executable(DolasTest)

file(GLOB_RECURSE SOURCES "*.cpp")
set(RENDER_SMOKE_TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/render/render_smoke_test.cpp")
list(REMOVE_ITEM SOURCES "${RENDER_SMOKE_TEST_SOURCE}")

target_sources(DolasTest PRIVATE ${SOURCES})

# 链接 DolasCore（被测模块）和 Catch2WithMain（提供 main 入口）
target_link_libraries(DolasTest PRIVATE DolasCore)
target_link_libraries(DolasTest PRIVATE DolasResource)
target_link_libraries(DolasTest PRIVATE Catch2::Catch2WithMain)
target_link_libraries(DolasTest PRIVATE reflectcpp::reflectcpp)

target_compile_features(DolasTest PRIVATE cxx_std_20)
dolas_enable_utf8(DolasTest)

# Windows 资源文件（图标等）
if(WIN32)
    target_sources(DolasTest PRIVATE ${CMAKE_SOURCE_DIR}/rc/Dolas.rc)
endif()

# IDE 文件夹归类
set_target_properties(DolasTest PROPERTIES FOLDER "EngineTest")

# 将 Catch2 extras 目录加入模块搜索路径，以便使用 catch_discover_tests
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/third_party/Catch2/extras)

# 使用 Catch2 提供的 CMake 辅助模块自动发现测试用例并注册到 CTest
include(Catch)
# DISCOVERY_MODE PRE_TEST 选项使测试发现在运行时而非构建时进行，避免构建时 DLL 依赖问题
catch_discover_tests(DolasTest
    DISCOVERY_MODE PRE_TEST
)

if(WIN32)
    add_executable(DolasRenderSmokeTest)
    target_sources(DolasRenderSmokeTest PRIVATE ${RENDER_SMOKE_TEST_SOURCE})

    target_link_libraries(DolasRenderSmokeTest PRIVATE DolasFunction)
    target_link_libraries(DolasRenderSmokeTest PRIVATE DolasPlatform)
    target_link_libraries(DolasRenderSmokeTest PRIVATE DolasCore)
    target_link_libraries(DolasRenderSmokeTest PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(DolasRenderSmokeTest PRIVATE d3d12.lib dxgi.lib dxguid.lib)

    target_compile_features(DolasRenderSmokeTest PRIVATE cxx_std_20)
    dolas_enable_utf8(DolasRenderSmokeTest)
    target_sources(DolasRenderSmokeTest PRIVATE ${CMAKE_SOURCE_DIR}/rc/Dolas.rc)
    set_target_properties(DolasRenderSmokeTest PROPERTIES FOLDER "EngineTest")

    add_test(
        NAME render_smoke
        COMMAND DolasRenderSmokeTest "Windows renderer initializes and renders one frame"
    )
    set_tests_properties(render_smoke PROPERTIES
        LABELS "render;smoke"
    )
endif()

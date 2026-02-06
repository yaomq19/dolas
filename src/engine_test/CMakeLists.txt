# ============ Catch2 + CTest 测试项目 ============
add_executable(DolasTest)

file(GLOB_RECURSE SOURCES "*.cpp")

target_sources(DolasTest PRIVATE ${SOURCES})

# 链接 DolasCore（被测模块）和 Catch2WithMain（提供 main 入口）
target_link_libraries(DolasTest PRIVATE DolasCore Catch2::Catch2WithMain)

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

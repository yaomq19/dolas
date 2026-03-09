add_subdirectory(engine_runtime)
add_subdirectory(engine_tool)
add_subdirectory(engine_test)

# ============ 文档文件配置 ============
# 自动扫描 docs 目录下的所有文档文件
file(GLOB DOCUMENTATION_FILES
    ${CMAKE_SOURCE_DIR}/docs/*.md
)

# 为文档文件创建源组，在IDE中组织显示
source_group("Documentation" FILES ${DOCUMENTATION_FILES})

# 创建静态库以在IDE中显示文档文件
add_library(Documentation STATIC ${DOCUMENTATION_FILES})
set_target_properties(Documentation PROPERTIES FOLDER "Documentation")
set_target_properties(Documentation PROPERTIES LINKER_LANGUAGE CXX)
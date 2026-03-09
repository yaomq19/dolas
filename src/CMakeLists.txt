add_subdirectory(engine_runtime)
add_subdirectory(engine_tool)
add_subdirectory(engine_test)

# ============ 文档文件配置 ============
# 将文档文件添加到项目中，以便在IDE中显示
set(DOCUMENTATION_FILES
    ${CMAKE_SOURCE_DIR}/AGENTS.md
    ${CMAKE_SOURCE_DIR}/README.md
)

# 为文档文件创建源组，在IDE中组织显示
source_group("Documentation" FILES ${DOCUMENTATION_FILES})

# 创建静态库以在IDE中显示文档文件
add_library(Documentation STATIC ${DOCUMENTATION_FILES})
set_target_properties(Documentation PROPERTIES FOLDER "Documentation")
set_target_properties(Documentation PROPERTIES LINKER_LANGUAGE CXX)
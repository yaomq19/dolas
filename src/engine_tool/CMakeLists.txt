# 工具目录：包含所有开发工具子项目

# 添加 Dolas 编辑器子目录
add_subdirectory(dolas_editor)
# IDE 文件夹组织
set_target_properties(DolasEditor PROPERTIES FOLDER "EngineTool")

# 添加着色器编译器子目录
add_subdirectory(shader_compiler)
# IDE 文件夹组织
set_target_properties(ShaderCompiler PROPERTIES FOLDER "EngineTool")

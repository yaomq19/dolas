# 模型导入器：这是一个独立的 C# WPF 工程（.csproj）。
# 我们只在 Visual Studio 生成器下把它“挂进解决方案”，不参与 CMake 的编译流水线。

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # 源码目录（仓库内），生成的 csproj 会引用这里的 *.cs / *.xaml
    set(MODEL_IMPORTER_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/ModelImporter")

    # 生成目录（build 内），避免把 .csproj 放进仓库
    set(MODEL_IMPORTER_GEN_DIR "${CMAKE_BINARY_DIR}/generated/model_importer/ModelImporter")
    file(MAKE_DIRECTORY "${MODEL_IMPORTER_GEN_DIR}")

    # 把路径转换成 MSBuild 更稳定识别的格式（用 /）
    file(TO_CMAKE_PATH "${MODEL_IMPORTER_SOURCE_DIR}" MODEL_IMPORTER_SOURCE_DIR)

    set(MODEL_IMPORTER_CSPROJ "${MODEL_IMPORTER_GEN_DIR}/ModelImporter.csproj")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/ModelImporter/ModelImporter.csproj.in"
        "${MODEL_IMPORTER_CSPROJ}"
        @ONLY
    )

    # 自动化解决 C# 包引用问题：配置完成后立即执行 NuGet 还原
    message(STATUS "Restoring NuGet packages for ModelImporter...")
    execute_process(
        COMMAND dotnet restore "${MODEL_IMPORTER_CSPROJ}"
        WORKING_DIRECTORY "${MODEL_IMPORTER_GEN_DIR}"
        RESULT_VARIABLE restore_result
    )
    if(NOT restore_result EQUAL 0)
        message(WARNING "Failed to restore NuGet packages for ModelImporter. You may need to restore manually in VS.")
    endif()

    if(COMMAND include_external_msproject)
        # 最简方式：仅把 csproj 挂进 VS 解决方案
        include_external_msproject(ModelImporter "${MODEL_IMPORTER_CSPROJ}")

        # 在 VS 解决方案里分组显示
        set_property(TARGET ModelImporter PROPERTY FOLDER "tools")
    else()
        message(WARNING "CMake is too old: include_external_msproject() is not available; skip adding WPF project to the solution.")
    endif()
endif()


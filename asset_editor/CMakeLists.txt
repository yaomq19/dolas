# 资产编辑器：这是一个独立的 C# WPF 工程（.csproj）。
# 我们只在 Visual Studio 生成器下把它“挂进解决方案”，不参与 CMake 的编译流水线。

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # 源码目录（仓库内），生成的 csproj 会引用这里的 *.cs / *.xaml
    set(DOLAS_ASSET_EDITOR_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/DolasAssetEditor")

    # 生成目录（build 内），避免把 .csproj 放进仓库
    set(DOLAS_ASSET_EDITOR_GEN_DIR "${CMAKE_BINARY_DIR}/generated/asset_editor/DolasAssetEditor")
    file(MAKE_DIRECTORY "${DOLAS_ASSET_EDITOR_GEN_DIR}")

    # 把路径转换成 MSBuild 更稳定识别的格式（用 /）
    file(TO_CMAKE_PATH "${DOLAS_ASSET_EDITOR_SOURCE_DIR}" DOLAS_ASSET_EDITOR_SOURCE_DIR)

    set(DOLAS_ASSET_EDITOR_CSPROJ "${DOLAS_ASSET_EDITOR_GEN_DIR}/DolasAssetEditor.csproj")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/DolasAssetEditor/DolasAssetEditor.csproj.in"
        "${DOLAS_ASSET_EDITOR_CSPROJ}"
        @ONLY
    )

    if(COMMAND include_external_msproject)
        # 最简方式：仅把 csproj 挂进 VS 解决方案，不传任何 GUID 参数
        include_external_msproject(DolasAssetEditor "${DOLAS_ASSET_EDITOR_CSPROJ}")

        # 在 VS 解决方案里分组显示到 DolasEngine 文件夹
        set_property(TARGET DolasAssetEditor PROPERTY FOLDER "DolasEngine")
    else()
        message(WARNING "CMake is too old: include_external_msproject() is not available; skip adding WPF project to the solution.")
    endif()
endif()


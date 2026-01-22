# ImGui 迁移到 Git Submodule 完成报告

## 迁移概述

已成功将 ImGui 从 vcpkg 依赖迁移到 git submodule，使用 yaomq19 的 docking 分支。

## 完成的修改

### 1. 添加 Git Submodule
```bash
git submodule add https://github.com/yaomq19/imgui.git third_party/imgui
```

**位置**: `third_party/imgui/`  
**分支**: master (包含 docking 和 multi-viewport 功能)

### 2. 创建 Third Party CMakeLists.txt
**文件**: `third_party/CMakeLists.txt`

**构建配置**:
- ImGui 核心文件 (imgui.cpp, imgui_demo.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp)
- ImGui 后端文件 (imgui_impl_win32.cpp, imgui_impl_dx11.cpp)
- 构建为静态库 `imgui`
- 公开包含目录: `third_party/imgui/` 和 `third_party/imgui/backends/`
- 自动链接系统库: `d3d11`, `dxgi`, `d3dcompiler`, `dwmapi`

### 3. 修改根 CMakeLists.txt
**文件**: `CMakeLists.txt`

**改动**:
```cmake
# 添加第三方库子目录（优先构建）
add_subdirectory(third_party)
```

### 4. 修改 Source CMakeLists.txt
**文件**: `source/CMakeLists.txt`

**改动**:
- 移除 `find_package(imgui CONFIG REQUIRED)`
- 将 `target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)` 改为 `imgui`

### 5. 更新 vcpkg.json
**文件**: `vcpkg.json`

**改动**:
- 移除 ImGui 依赖项（包括 dx11-binding 和 win32-binding features）

### 6. 创建文档
**文件**: `third_party/README.md`

包含：
- 子模块使用说明
- 初始化和更新指南
- 构建配置说明
- 未来迁移计划

## 验证结果

### ✅ 编译成功
```
imgui.vcxproj -> C:\repos\dolas\build\lib\Debug\imgui.lib
Dolas.vcxproj -> C:\repos\dolas\build\bin\Debug\Dolas.exe
```

### ✅ vcpkg 自动移除
```
The following packages will be removed:
    imgui:x64-windows
Removing 1/1 imgui:x64-windows
```

### ✅ ImGui 功能完整
- 核心 ImGui 功能
- Win32 后端 (窗口、输入)
- DX11 后端 (渲染)
- Docking 支持 (通过 yaomq19 分支)
- Multi-Viewport 支持

## 使用方式

### 首次克隆后初始化
```bash
git submodule update --init --recursive
```

### 更新 ImGui 到最新版本
```bash
cd third_party/imgui
git pull origin master
cd ../..
git add third_party/imgui
git commit -m "Update ImGui submodule"
```

### CMake 配置
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<vcpkg_toolchain_path>
```

### 构建
```bash
cmake --build build --config Debug --target Dolas
```

## 优势

1. **版本控制精确**: 锁定 ImGui 的具体提交，团队成员使用完全相同的版本
2. **启用 Docking**: yaomq19 分支默认启用 docking 和 multi-viewport 功能
3. **离线构建**: 不依赖 vcpkg 网络下载 ImGui
4. **本地修改**: 可以在 submodule 中进行本地修改和调试
5. **构建透明**: 所有 ImGui 源码可见，方便学习和调试

## 后续计划

按照同样的方式，逐步迁移其他库：

- [ ] spdlog
- [ ] tinyxml2
- [ ] assimp
- [ ] directxtex
- [ ] tracy

每个库都将：
1. 作为 git submodule 添加到 `third_party/`
2. 在 `third_party/CMakeLists.txt` 中配置构建
3. 从 `vcpkg.json` 移除依赖
4. 更新 `source/CMakeLists.txt` 的链接方式

## 注意事项

### Git Submodule 常见操作

**克隆包含子模块的仓库**:
```bash
git clone --recursive <repo_url>
```

**更新所有子模块**:
```bash
git submodule update --remote --merge
```

**查看子模块状态**:
```bash
git submodule status
```

### CMake 构建顺序

CMake 会按以下顺序构建：
1. `third_party/imgui` → `imgui.lib`
2. `precompile` → `Precompile.exe` (生成 RSD 头文件)
3. `source` → `Dolas.exe` (链接 imgui.lib)

### IDE 项目结构

在 Visual Studio 中，ImGui 会出现在 "Third Party" 文件夹中，保持项目结构清晰。

## 遇到问题？

### Q: 子模块目录为空
A: 运行 `git submodule update --init --recursive`

### Q: 编译找不到 imgui.h
A: 检查 `third_party/imgui/` 目录是否存在且有内容

### Q: 链接错误 LNK2019
A: 确保 `imgui` 目标在 `Dolas` 之前构建（CMake 已配置依赖关系）

### Q: 想使用原版 ImGui（非 docking）
A: 修改 `.gitmodules` 中的 URL 为 `https://github.com/ocornut/imgui.git`

## 总结

ImGui 已成功从 vcpkg 迁移到 git submodule，项目现在：
- ✅ 使用 yaomq19 的 docking 分支
- ✅ 不再依赖 vcpkg 的 ImGui 包
- ✅ 完全离线构建 ImGui
- ✅ 保持所有功能正常（包括 docking 和 multi-viewport）
- ✅ 编译和运行正常

下一步可以继续迁移其他库，最终实现完全不依赖 vcpkg 的目标。

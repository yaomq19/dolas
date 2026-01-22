# 使用 Git Submodule 的 ImGui - 快速开始

## 新成员克隆项目

如果你是第一次克隆这个项目：

```bash
# 方法 1: 克隆时同时初始化子模块（推荐）
git clone --recursive https://github.com/your-org/dolas.git

# 方法 2: 先克隆，再初始化子模块
git clone https://github.com/your-org/dolas.git
cd dolas
git submodule update --init --recursive
```

## 已有项目成员更新

如果你之前已经克隆了项目，现在要同步 ImGui submodule：

```bash
# 拉取最新代码
git pull

# 初始化并更新子模块
git submodule update --init --recursive
```

## 构建项目

```bash
# 配置 CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<your_vcpkg_toolchain_path>

# 构建项目
cmake --build build --config Debug

# 或者在 Visual Studio 中打开 build/Dolas.sln
```

## 验证 ImGui 正常工作

1. 运行 `Dolas.exe`
2. 按 F11 打开 Debug Tools 窗口
3. 查看 "3D Viewport" 窗口（显示网格占位符和鼠标指示器）
4. 菜单栏应该正常显示

## 常见问题

### Q: third_party/imgui 目录为空
**A**: 运行 `git submodule update --init --recursive`

### Q: 编译错误：找不到 imgui.h
**A**: 
1. 确认 `third_party/imgui/` 目录存在且有内容
2. 重新运行 CMake 配置
3. 检查是否有 `imgui.lib` 在 `build/lib/Debug/` 中

### Q: 如何更新 ImGui 到最新版本
**A**:
```bash
cd third_party/imgui
git pull origin master
cd ../..
git add third_party/imgui
git commit -m "Update ImGui to latest version"
```

### Q: 想切换回 vcpkg 的 ImGui
**A**: 
1. 在 `vcpkg.json` 中恢复 ImGui 依赖
2. 在 `source/CMakeLists.txt` 中恢复 `find_package(imgui CONFIG REQUIRED)`
3. 移除 `third_party/` 相关的构建配置

## 技术细节

### ImGui 构建配置
- **目标**: `imgui` (静态库)
- **源文件**: 核心 + Demo + Draw + Tables + Widgets
- **后端**: Win32 + DirectX 11
- **功能**: Docking + Multi-Viewport (yaomq19 分支)
- **输出**: `build/lib/Debug/imgui.lib`

### 依赖链
```
Dolas.exe
  └── imgui.lib (third_party/imgui)
      └── d3d11.lib, dxgi.lib, d3dcompiler.lib, dwmapi.lib
```

### Submodule 信息
- **URL**: https://github.com/yaomq19/imgui.git
- **路径**: third_party/imgui
- **分支**: master (docking)
- **提交**: 由 .gitmodules 和 git 索引跟踪

## 下一步

你现在可以：
1. 开始开发功能，ImGui 已经可以正常使用
2. 探索 yaomq19 分支的 docking 和 multi-viewport 功能
3. 查看 `docs/IMGUI_VIEWPORT_INTEGRATION.md` 了解如何使用视口功能
4. 继续迁移其他库到 git submodule（参考 `third_party/README.md`）

## 有问题？

- 查看 `docs/IMGUI_SUBMODULE_MIGRATION.md` 了解完整迁移过程
- 查看 `third_party/README.md` 了解 submodule 管理
- 在项目 Issue 中提问

# Dolas Asset Editor（WPF）

这是一个独立的 C# WPF 工程，用作 Dolas 游戏引擎的**资产编辑器基础框架**。

## 目录

- `DolasAssetEditor/`：WPF 应用（.csproj）

## 与 CMake / VS 解决方案的关系

该工程通过 `asset_editor/CMakeLists.txt` 内的 `include_external_msproject()` 在 **Visual Studio 生成器** 下被加入到根解决方案中：

- 不会改变现有 C++ 目标的构建方式
- 仅用于在 VS 中“同一个解决方案里”便于一起开发



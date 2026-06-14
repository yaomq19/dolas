# macOS Vulkan/MoltenVK 构建运行说明

## 依赖

使用 Homebrew 安装当前 macOS Vulkan bring-up 所需依赖：

```bash
brew install glfw vulkan-headers vulkan-loader vulkan-tools molten-vk glslang
```

验证工具可见：

```bash
which vulkaninfo
which glslangValidator
pkg-config --modversion glfw3
```

验证 MoltenVK/Metal 可用：

```bash
vulkaninfo --summary
```

如果这里出现 `VK_ERROR_INCOMPATIBLE_DRIVER` 或 `MoltenVK requires Metal, which is not available on this device`，说明当前 Mac、虚拟机或登录会话没有可用 Metal 设备；项目可以编译，但 Vulkan/MoltenVK 窗口无法真正运行显示。

## 配置、构建、测试

```bash
cmake --preset macos-vulkan-debug
cmake --build --preset macos-vulkan-debug -j8
ctest --preset macos-vulkan-debug --output-on-failure
```

产物位于：

```bash
build/macos-vulkan-debug/bin/DolasEditor
build/macos-vulkan-debug/bin/ShaderCompiler
build/macos-vulkan-debug/bin/DolasTest
```

## Shader 编译

macOS 下 `ShaderCompiler` 使用 `glslangValidator` 将 HLSL 编译为 SPIR-V，输出到：

```bash
build/macos-vulkan-debug/compiled_shaders/
```

运行方式：

```bash
./build/macos-vulkan-debug/bin/ShaderCompiler
```

在交互提示中输入 `all` 编译全部 `content/shader/**/*.hlsl`。

`DolasEditor` 构建时也会生成运行时需要的 SPIR-V：

```bash
build/macos-vulkan-debug/runtime_shaders/debug_draw_vs.spv
build/macos-vulkan-debug/runtime_shaders/debug_draw_ps.spv
build/macos-vulkan-debug/runtime_shaders/opaque_vs.spv
build/macos-vulkan-debug/runtime_shaders/opaque_ps.spv
build/macos-vulkan-debug/runtime_shaders/deferred_shading_vs.spv
build/macos-vulkan-debug/runtime_shaders/deferred_shading_ps.spv
build/macos-vulkan-debug/runtime_shaders/sky_box_vs.spv
build/macos-vulkan-debug/runtime_shaders/sky_box_ps.spv
build/macos-vulkan-debug/runtime_shaders/present_scene_ps.spv
```

macOS Vulkan editor 的运行时 `deferred_shading_ps.spv` 由 `content/shader/deferred_shading/deferred_shading_macos_ps.hlsl` 生成，用于在 GBuffer clear 深度处输出程序化 sky/ground 背景；原 `deferred_shading_ps.hlsl` 不变，保留给现有路径。

## 运行编辑器

```bash
./build/macos-vulkan-debug/bin/DolasEditor
```

当前 macOS 入口是 GLFW + Vulkan + ImGui bring-up 路径，用于验证窗口、swapchain、depth attachment、GBuffer A/B/C/D + depth offscreen attachments、SceneResult offscreen target、opaque GBuffer draw、deferred shading fullscreen draw、sky/ground 背景、debug draw overlay、SceneResult 到 swapchain present 和 ImGui 后端。编辑器窗口按参考 Windows editor 截图组织为左侧 Scene Hierarchy、中间 Viewport、右侧 Properties、底部 Content Browser，Viewport 中直接显示 SceneResult。材质贴图缺失时会创建 1x1 fallback albedo/normal texture，避免阻塞启动和渲染。Windows 仍使用原有 `vs2022-*` preset、Win32/D3D 路径和 `DolasFunction` 入口。

当前本机截图验证输出为 `/private/tmp/dolas_editor_current.png`，参考 Windows editor 截图为 `docs/images/editor_screenshot.png`。

## Windows 回归验证

本机 macOS 无法直接运行 Visual Studio generator；Windows 侧最终回归需要在 Windows 环境执行：

```powershell
cmake --preset vs2022-debug
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug --output-on-failure
.\build\vs2022-debug\bin\Debug\DolasEditor.exe
```

预期保持不变：

- Windows 默认 `DOLAS_RENDER_BACKEND` 为 `D3D12`。
- `DolasEditor` 使用 `src/engine_tool/dolas_editor/main.cpp`、`DolasFunction` 和 Win32 `.rc` 资源。
- ImGui 使用 Win32 + DX12 backend。
- DirectXTex 和 D3D/DXGI/D3DCompiler 库只在 `WIN32` 分支链接。
- macOS 专用 `main_macos_vulkan.cpp`、`deferred_shading_macos_ps.hlsl` 和 `present_scene_ps.hlsl` 不进入 Windows editor 运行路径。

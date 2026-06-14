# macOS Vulkan/MoltenVK 适配计划

## 目标

让当前 Dolas 项目可以在本机 macOS 上编译、启动并运行 `DolasEditor`。

最终验收标准：

- macOS Debug 构建可以完成 CMake configure、build 和 CTest。
- `DolasEditor` 可以启动 GLFW 窗口。
- 图形后端使用 Vulkan，并通过 MoltenVK 运行在 macOS Metal 之上。
- 编辑器 ImGui 界面可以显示。
- 默认场景可以完成基础渲染，包括 GBuffer、DeferredShading、Skybox、Debug/ImGui、Present 这条主路径。
- Windows 当前 D3D 路径继续可构建，不因 macOS 适配被破坏。

## 固定决策

- macOS 窗口和事件层使用 GLFW。
- macOS 图形 API 使用 Vulkan；macOS 上的 Vulkan 通过 MoltenVK/Metal 适配层运行。
- 保留现有 HLSL shader 内容，不改写为 GLSL。
- Vulkan shader 链路使用 HLSL 到 SPIR-V，优先使用 Vulkan SDK 提供的 DXC/glslang 工具。
- macOS 第一版以编辑器和默认场景跑通为目标，不追求完整功能 parity。
- 缺失的材质贴图不能阻塞启动和渲染；使用 fallback texture 并输出 warning。

## 当前执行结果

截至 2026-06-14，本机 macOS Vulkan bring-up 已通过以下验证：

- `cmake --preset macos-vulkan-debug`
- `cmake --build --preset macos-vulkan-debug -j8`
- `ctest --preset macos-vulkan-debug --output-on-failure`：148/148 通过
- `cmake --preset macos-vulkan-release`
- `cmake --build --preset macos-vulkan-release -j8`
- `ctest --preset macos-vulkan-release --output-on-failure`：148/148 通过
- `./build/macos-vulkan-debug/bin/ShaderCompiler` 交互输入 `all`：16/16 HLSL 成功编译到 SPIR-V
- `./build/macos-vulkan-debug/bin/DolasEditor` 可启动 GLFW + Vulkan/MoltenVK 窗口，显示 ImGui 四区布局和 SceneResult viewport
- 当前 macOS 截图验证输出：`/private/tmp/dolas_editor_current.png`
- Windows 参考截图：`docs/images/editor_screenshot.png`

当前 macOS editor 运行路径覆盖：

- Vulkan instance/device/swapchain
- GBuffer A/B/C/D + depth offscreen attachments
- SceneResult offscreen color target
- opaque GBuffer draw
- deferred shading fullscreen draw
- sky/ground 背景
- debug draw overlay
- SceneResult 到 swapchain present
- ImGui GLFW + Vulkan 后端
- 缺失材质贴图的 fallback texture

Windows 影响控制：

- Windows 默认 `DOLAS_RENDER_BACKEND` 仍为 `D3D12`。
- Windows `vs2022-*` preset 未切到 Vulkan。
- `DolasEditor` 在 `WIN32` 分支仍使用 `main.cpp`、`DolasFunction` 和 Win32 `.rc` 资源。
- macOS editor 入口 `main_macos_vulkan.cpp` 只在 `APPLE AND DOLAS_RENDER_BACKEND STREQUAL "Vulkan"` 分支编译。
- 原 `content/shader/deferred_shading/deferred_shading_ps.hlsl` 未改动；macOS runtime 使用新增 `deferred_shading_macos_ps.hlsl` 生成自己的 SPIR-V。
- 本机无法实测 Windows 构建；当前结论基于条件编译、preset 审计、CMake Windows 分支配置探针和 macOS Debug/Release 构建测试。

Windows 分支配置探针：

```bash
cmake -S . -B build/windows-config-probe -G "Unix Makefiles" -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY -DCMAKE_BUILD_TYPE=Debug
```

探针结果：

- configure/generate 成功。
- `build/windows-config-probe/CMakeCache.txt` 中 `DOLAS_RENDER_BACKEND=D3D12`、`ENABLE_VULKAN=OFF`。
- 生成的 `DolasEditor` 目标源文件是 `src/engine_tool/dolas_editor/main.cpp`。
- 生成的 `DolasEditor` 链接依赖包含 `DolasFunction`、DirectXTex、D3D12、D3D11、DXGI、D3DCompiler、DXGUID 和 Imm32。
- 该探针只证明 CMake Windows 分支可解析并选择了原 Windows/D3D 路径；因为本机没有 Visual Studio generator、Windows SDK 和 Windows 运行环境，它不能替代 Windows 实机 configure/build/test/run 验收。

## 初始已知阻塞

- 顶层 CMake 目前只在 Windows 分支里查找 Vulkan，macOS `ENABLE_VULKAN=ON` 不会正确建立 Vulkan 构建链。
- `third_party/DirectXTex` 被无条件加入构建，macOS configure 当前会停在 `directxmath` / `directx-headers` 查找失败。
- 多个 public header 直接暴露 `Windows.h`、`d3d12.h`、`dxgiformat.h`、`HWND`、`DXGI_FORMAT`、`ID3D12Resource` 等 Windows/D3D 类型。
- `DolasPlatform` 无条件链接 `d3d12.lib dxgi.lib dxguid.lib`。
- `DolasFunction` 里的 RHI、Texture、Buffer、Shader、ImGui、Input 仍与 D3D11/D3D12/Win32 强耦合。
- 现有 shader 在运行时用 `D3DCompileFromFile` 编译，并依赖 D3D reflection。
- 本机当前未发现 `VULKAN_SDK`、`vulkaninfo`、`dxc`、`glslangValidator`。
- 默认场景引用的 `content/material/textures/mace01_*.png` 当前不在仓库中。

## 实施步骤

### 1. 构建系统与依赖隔离

- 新增 macOS Vulkan CMake preset：
  - `macos-vulkan-debug`
  - `macos-vulkan-release`
- 增加渲染后端选项，例如 `DOLAS_RENDER_BACKEND=Vulkan`，Windows 默认保留当前 D3D 路径。
- 在 macOS Vulkan preset 中查找：
  - Vulkan SDK
  - GLFW
  - shader 编译工具 DXC 或 glslang
- 把 Windows-only 依赖全部放入 `if(WIN32)`：
  - `/utf-8`
  - `d3d11.lib`
  - `d3d12.lib`
  - `dxgi.lib`
  - `dxguid.lib`
  - `d3dcompiler.lib`
  - `imm32.lib`
  - Windows `.rc` 资源文件
  - DirectXTex
- macOS 不构建 DirectXTex；纹理加载改走跨平台路径。
- CMake 在缺少 Vulkan SDK、GLFW 或 shader 工具时给出清晰错误信息和安装提示。

### 2. 平台层抽象

- 从 public header 中移除 Win32 类型暴露。
- 新增或改造跨平台窗口接口，例如 `DolasWindow`：
  - `Initialize(width, height, title)`
  - `PollEvents()`
  - `ShouldClose()`
  - `GetFramebufferSize()`
  - `CreateVulkanSurface(VkInstance)`
  - `GetNativeHandle()`
- Windows 实现继续使用 Win32。
- macOS 实现使用 GLFW：
  - 设置 `GLFW_CLIENT_API = GLFW_NO_API`
  - 创建 Vulkan surface
  - 处理窗口关闭、resize、鼠标、键盘、滚轮事件
- `DolasEngine::Run()` 改为调用平台窗口抽象的事件轮询，而不是直接使用 `PeekMessage`。
- `InputManager` 改为接收平台无关的 key/mouse 事件；保留 W/A/S/D、Space、Shift、F11、右键捕获、鼠标移动和滚轮。

### 3. RHI 后端边界

- 保留 `DolasRHI` 作为 engine-facing facade。
- 在内部拆分后端：
  - Windows：`D3D12RHIBackend`
  - macOS：`VulkanRHIBackend`
- public RHI 类型不再暴露 D3D 原生句柄。
- 用平台无关类型替换当前 public API 中的 D3D 类型：
  - texture handle
  - buffer handle
  - shader bytecode view
  - render target view
  - depth stencil view
  - descriptor/sampler binding
  - texture format
- D3D 原生句柄和 Vulkan 原生句柄只保留在对应 backend 的 private 实现里。
- 第一版 Vulkan backend 至少支持：
  - instance/device/physical device/queue
  - macOS portability extension
  - GLFW Vulkan surface
  - swapchain
  - command pool / command buffer
  - fence / semaphore
  - render pass/framebuffer 或 dynamic rendering
  - viewport/scissor
  - clear RTV/DSV
  - render target/depth texture
  - vertex/index/constant/storage buffer
  - sampled texture
  - sampler
  - descriptor set/layout/pool
  - graphics pipeline cache
  - indexed draw
  - present

### 4. Shader 编译与反射

- 保留 `content/shader/**/*.hlsl`。
- 为 Vulkan 增加 HLSL 到 SPIR-V 编译路径。
- 编译参数需要保持现有寄存器布局：
  - `b0`：PerView
  - `b1`：PerFrame
  - `b2`：PerObject
  - `b13`：GlobalConstants
  - `t0..t15`：Texture/SRV
  - `s0..s15`：Sampler
- 优先使用 DXC 参数：
  - `-spirv`
  - `-fvk-use-dx-layout`
  - `-E VS/PS`
  - `-T vs_6_0/ps_6_0`
  - include 目录指向 `content/shader`
- 生成 SPIR-V 后缓存到构建目录或内容派生目录，避免每帧运行时重复编译。
- 用 SPIR-V reflection 替代 D3D reflection，至少支持：
  - constant buffer 名称、大小、变量 offset、变量 size
  - texture/sampler binding
  - vertex input layout 所需信息
- `ShaderContext` 存储后端无关 bytecode 和 reflection 数据；D3D blob 只保留在 Windows backend。

### 5. 纹理和资源路径

- macOS 纹理加载不依赖 DirectXTex。
- 使用跨平台图片加载实现 PNG/HDR：
  - PNG：可复用仓库已有第三方中的 `stb_image.h`，或新增明确的 third_party/stb 集成。
  - HDR：使用 `stb_image` float 加载路径。
- 第一版必须支持：
  - `R8G8B8A8_UNORM`
  - `R8G8B8A8_SRGB`
  - `R16G16B16A16_FLOAT` 或 `R32G32B32A32_FLOAT`
  - `D24_UNORM_S8_UINT` 或 Vulkan 兼容深度模板格式
  - typeless 深度格式到 Vulkan 深度格式的映射
- 默认场景缺失 PNG 时创建 fallback 1x1 texture：
  - albedo fallback：白色
  - normal fallback：法线默认值
  - roughness/metallic fallback：合理默认值
- `PathUtils::SetProjectDirectoryPath("hack_path")` 后续应替换为可从可执行文件路径或 CMake 定义推导的项目/content 路径。

### 6. ImGui 适配

- Windows 保留 `imgui_impl_win32.cpp` + `imgui_impl_dx12.cpp`。
- macOS Vulkan 使用：
  - `imgui_impl_glfw.cpp`
  - `imgui_impl_vulkan.cpp`
- `ImGuiManager` 不再在 public API 中暴露 `ID3D12GraphicsCommandList`。
- 增加 backend-neutral 的 ImGui render hook，由当前 RHI backend 提供绘制所需对象。
- macOS 第一版确保：
  - DockSpace 显示
  - Scene viewport 尺寸计算正常
  - Content browser/Hierarchy/Properties 面板不崩溃
  - ImGui draw data 在 swapchain render pass 中渲染

### 7. 默认场景渲染路径

- Vulkan backend 需要覆盖当前默认渲染管线所需能力：
  - `BeginFrame`
  - `UpdatePerFrameParameters`
  - `UpdatePerViewParameters`
  - `UpdatePerObjectParameters`
  - `ClearRenderTargetView`
  - `ClearDepthStencilView`
  - `SetRenderTargetViewAndDepthStencilView`
  - `SetRenderTargetViewWithoutDepthStencilView`
  - `SetViewPort`
  - `SetRasterizerState`
  - `SetDepthStencilState`
  - `SetBlendState`
  - `BindVertexContext`
  - `BindPixelContext`
  - `DrawRenderPrimitive`
  - `Present`
- 默认 render resource 需要映射：
  - GBuffer A/B/C/D：`R8G8B8A8_UNORM`
  - DepthStencil：当前 `R24G8_TYPELESS`，Vulkan 后端映射到可用深度/模板格式
  - SceneResult：`R8G8B8A8_UNORM`
- `Present` 继续把 scene result 合成到 backbuffer，再绘制 ImGui。

## 验证流程

### 配置验证

```bash
cmake --preset macos-vulkan-debug
```

期望结果：

- CMake configure 成功。
- 如果 Vulkan SDK 或 GLFW 缺失，错误信息明确说明缺什么以及如何安装。

### 构建验证

```bash
cmake --build --preset macos-vulkan-debug -j8
```

期望结果：

- `DolasEditor` 构建成功。
- `ShaderCompiler` 构建成功。
- `DolasTest` 构建成功。

### 测试验证

```bash
ctest --preset macos-vulkan-debug --output-on-failure
```

期望结果：

- 现有 math/path/asset/hash 测试通过。
- macOS 平台兼容改造不破坏现有非图形单元测试。

### Shader 验证

- 编译 `content/shader/**/*.hlsl` 到 SPIR-V。
- 验证至少以下 shader 可以通过：
  - `blinn_phong_vs.hlsl`
  - `blinn_phong_ps.hlsl`
  - `opaque_vs.hlsl`
  - `opaque_ps.hlsl`
  - `deferred_shading_vs.hlsl`
  - `deferred_shading_ps.hlsl`
  - `sky_box_vs.hlsl`
  - `sky_box_ps.hlsl`
  - `debug_draw_vs.hlsl`
  - `debug_draw_ps.hlsl`
- 验证 `GlobalConstants` reflection 能正确找到变量 offset。

### 运行验证

```bash
./build/macos-vulkan-debug/bin/DolasEditor
```

期望结果：

- GLFW 窗口出现。
- Vulkan instance/device/swapchain 创建成功。
- ImGui 主界面出现。
- 默认场景可以显示基础几何和 skybox。
- 右键捕获鼠标、W/A/S/D、Space、Shift、滚轮、F11 等核心交互不崩溃。
- 关闭窗口可以干净退出，没有明显 GPU validation error。

### Windows 回归验证

```bash
cmake --preset vs2022-debug
cmake --build --preset vs2022-debug -j16
```

期望结果：

- Windows preset 仍能 configure/build。
- D3D backend 仍保留。

## 不在第一版范围内

- 不重写全部渲染架构。
- 不追求 macOS 与 Windows 完整视觉一致。
- 不实现完整 DDS/BC 压缩纹理链路，除非默认场景必须依赖。
- 不实现 Metal 原生后端。
- 不改写 HLSL 为 GLSL。
- 不引入 Slang 作为第一版 shader 语言。
- 不做多线程命令录制。
- 不做完整资源生命周期重构，只做 macOS Vulkan bring-up 必需的边界整理。

## 完成定义

任务完成时必须满足：

- 新增文档或 README 说明 macOS 构建依赖、配置、构建、运行命令。
- `macos-vulkan-debug` preset 可用。
- `DolasEditor` 在 macOS 上可启动并渲染默认场景。
- `DolasTest` 可在 macOS 上运行。
- Windows-only 代码已被平台宏或 backend 边界隔离。
- Public headers 不再强制 macOS 编译器包含 Win32/D3D header。
- 缺失贴图不会导致启动失败。
- 最终提交说明列出已实现后端能力和仍未实现的限制。

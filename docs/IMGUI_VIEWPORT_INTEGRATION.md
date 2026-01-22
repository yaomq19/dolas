# ImGui 编辑器视口集成方案（方案 C）

## 概述

本文档说明如何将 Dolas 引擎集成 ImGui 编辑器风格的界面，包括：
- 启用 ImGui Docking 和多视口功能
- 创建主 DockSpace 作为编辑器主界面
- 在中间区域内嵌 3D 视口窗口，支持鼠标交互

## 修改内容

### 1. `ImGuiManager.h` - 添加视口相关接口

**新增成员变量：**
- `m_viewport_hovered` - 鼠标是否悬停在视口上
- `m_viewport_focused` - 视口是否获得焦点
- `m_viewport_size` - 视口尺寸
- `m_viewport_pos` - 视口屏幕位置

**新增公共方法：**
- `IsViewportHovered()` - 查询视口悬停状态
- `IsViewportFocused()` - 查询视口焦点状态

**新增私有方法：**
- `RenderMainDockSpace()` - 渲染主 DockSpace
- `RenderViewportWindow()` - 渲染 3D 视口窗口
- `RenderDebugToolsWindow()` - 渲染调试工具窗口

### 2. `ImGuiManager.cpp` - 实现编辑器布局

#### Initialize() 修改
```cpp
// 启用 Docking 和多视口
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

// 调整样式以适配多视口
ImGuiStyle& style = ImGui::GetStyle();
if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
{
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
}
```

#### Render() 重构
新的渲染流程：
1. 开始 ImGui 新帧
2. 渲染主 DockSpace（覆盖整个主窗口）
3. 渲染 3D 视口窗口（可停靠）
4. 渲染调试工具窗口（F11 切换）
5. 渲染 ImGui
6. **更新和渲染平台窗口（多视口支持）**

```cpp
// 多视口支持的关键代码
if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
{
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
```

#### RenderMainDockSpace() 实现
- 创建覆盖整个主窗口的 DockSpace
- 添加顶部菜单栏（File, View, Help）
- 所有窗口都可以停靠到这个 DockSpace

#### RenderViewportWindow() 实现
- 创建名为 "3D Viewport" 的窗口
- 获取可用区域作为视口尺寸
- 跟踪鼠标悬停和焦点状态
- **目前显示占位符网格和鼠标位置指示器**
- 预留了嵌入 3D 渲染纹理的接口

### 3. `dolas_rhi.cpp` - 窗口创建调整

保留了主窗口创建（ImGui 多视口需要），并添加注释说明：
```cpp
// 创建窗口（保持可见，因为 ImGui 主视口会使用它）
m_window_handle = CreateWindowW(L"D3DWndClassName", L"Dolas Engine - Main Window",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, NULL, 0);
```

## 当前功能

### ✅ 已实现
- [x] ImGui Docking 和多视口启用
- [x] 主 DockSpace 布局
- [x] 3D 视口窗口框架
- [x] 视口鼠标悬停/焦点检测
- [x] 调试工具窗口（F11 切换）
- [x] 顶部菜单栏
- [x] 视口占位符显示（网格 + 鼠标指示器）

### 🚧 待实现（下一步）

#### 1. 离屏渲染纹理集成
需要修改渲染管线，将 3D 场景渲染到纹理而非直接到 SwapChain：

```cpp
// 在 RenderPipelineManager 中
TextureID CreateViewportRenderTarget(UInt width, UInt height)
{
    // 创建 D3D11 渲染目标纹理
    // 创建对应的 RTV 和 SRV
    // 返回 TextureID
}

// 在 ImGuiManager::RenderViewportWindow() 中
ID3D11ShaderResourceView* viewport_srv = GetViewportTextureSRV();
if (viewport_srv && m_viewport_size.x > 0 && m_viewport_size.y > 0)
{
    ImGui::Image((ImTextureID)viewport_srv, m_viewport_size);
}
```

**步骤：**
1. 在 `RenderPipelineManager` 添加视口渲染目标创建函数
2. 监测视口尺寸变化，动态重建纹理
3. 修改渲染流程：先渲染到离屏纹理，再由 ImGui 显示
4. 在 `ImGuiManager` 中获取 SRV 并用 `ImGui::Image()` 显示

#### 2. 视口交互逻辑
当前已经能检测 `m_viewport_hovered` 和 `m_viewport_focused`，下一步需要：

```cpp
// 在 RenderViewportWindow() 中
if (m_viewport_hovered)
{
    ImGuiIO& io = ImGui::GetIO();
    
    // 鼠标点击
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        // 转换为视口局部坐标
        ImVec2 mouse_pos = io.MousePos;
        ImVec2 local_pos = ImVec2(mouse_pos.x - m_viewport_pos.x, 
                                   mouse_pos.y - m_viewport_pos.y);
        
        // TODO: 触发场景对象拾取
        // g_dolas_engine.m_scene_manager->PickObject(local_pos);
    }
    
    // 右键拖拽旋转相机
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        
        // TODO: 更新相机旋转
        // g_dolas_engine.m_camera_controller->Rotate(delta.x, delta.y);
        
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    }
    
    // 鼠标滚轮缩放
    if (io.MouseWheel != 0.0f)
    {
        // TODO: 更新相机距离
        // g_dolas_engine.m_camera_controller->Zoom(io.MouseWheel);
    }
}
```

**与现有 InputManager 的协调：**
- 视口交互用 ImGui 输入 API（`IsMouseClicked/Dragging/GetMouseWheel`）
- 全局快捷键仍用 `InputManager`（如 F11 切换调试窗口）
- 避免 `ImGui_ImplWin32_WndProcHandler` 返回 true 时，引擎侧接收不到鼠标事件

#### 3. 相机控制器
创建一个 `EditorCameraController` 类：

```cpp
class EditorCameraController
{
public:
    void OnMouseDrag(float delta_x, float delta_y);  // 右键拖拽旋转
    void OnMouseWheel(float delta);                   // 滚轮缩放
    void OnKeyPress(int key);                         // WASD 移动
    
    void Update(float delta_time);                    // 更新相机变换
    RenderCamera* GetCamera();                        // 获取编辑器相机
};
```

#### 4. 增强功能
- 添加更多停靠窗口：
  - 场景层级树（Scene Hierarchy）
  - 属性检查器（Properties）
  - 资源浏览器（Asset Browser）
  - 控制台（Console）
- 添加工具栏（Toolbar）
- 添加 Gizmo（平移/旋转/缩放工具）
- 保存/加载 DockSpace 布局

## 使用说明

### 运行效果
启动引擎后，你会看到：
1. 一个带菜单栏的主窗口（ImGui DockSpace）
2. 中间的 "3D Viewport" 窗口（目前显示网格占位符）
3. 按 F11 可切换 "Debug Tools" 窗口

### 窗口操作
- **拖动窗口标题栏**：可以在 DockSpace 中停靠
- **拖出窗口**：可以创建独立的操作系统窗口（多显示器支持）
- **菜单栏**：
  - File → Exit：退出引擎
  - View → Debug Tools：切换调试工具窗口（等同于 F11）
  - Help → About：待实现

### 视口交互（占位符阶段）
- 鼠标移到 3D Viewport 窗口内时，会显示黄色圆圈和坐标
- 视口尺寸和悬停/焦点状态会显示在 Debug Tools 窗口中

## 架构说明

### 渲染流程（当前）
```
Engine::Run()
  └─> TickManager::Tick()
       └─> RenderPipelineManager::Render()
            └─> 渲染到 SwapChain BackBuffer
       └─> ImGuiManager::Render()
            └─> 渲染 ImGui UI 到 BackBuffer
       └─> RHI::Present()
            └─> SwapChain::Present()
```

### 渲染流程（目标）
```
Engine::Run()
  └─> TickManager::Tick()
       └─> RenderPipelineManager::Render()
            ├─> 清空 BackBuffer
            ├─> 渲染 3D 场景到 ViewportRenderTarget（离屏纹理）
            │    └─> 转换 ViewportRenderTarget 状态为 SHADER_RESOURCE
            └─> 渲染 ImGui UI 到 BackBuffer
                 └─> ImGui::Image(ViewportRenderTarget) 显示 3D 场景
       └─> RHI::Present()
            └─> SwapChain::Present()
```

### InputManager 协调
- **视口内交互**：用 `ImGui::IsItemHovered()` + `ImGui::GetIO()` 捕获
- **全局快捷键**：继续用 `InputManager::ConsumeKeyEvent()`
- **避免冲突**：当 `ImGuiManager::IsViewportFocused()` 为 true 时，InputManager 不处理相机移动

## 测试清单

### 基础功能测试
- [ ] 启动引擎，主窗口正常显示
- [ ] DockSpace 正常工作，窗口可停靠
- [ ] 3D Viewport 窗口显示网格占位符
- [ ] 鼠标移到 Viewport 内，黄色指示器跟随
- [ ] F11 切换 Debug Tools 窗口
- [ ] 菜单栏 File → Exit 正常退出
- [ ] 拖动窗口到主窗口外，创建独立窗口
- [ ] FPS 显示正常更新

### 下一阶段测试（离屏渲染后）
- [ ] 3D 场景正确显示在 Viewport 中
- [ ] 调整 Viewport 尺寸，渲染纹理正确重建
- [ ] 鼠标在 Viewport 内拖拽，相机正确响应
- [ ] 鼠标滚轮缩放相机
- [ ] 点击 Viewport 内的物体，正确拾取

## 参考资料

- [ImGui Docking 分支文档](https://github.com/ocornut/imgui/wiki/Docking)
- [ImGui 多视口示例](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx11/main.cpp)
- [Unity/Unreal 编辑器布局参考](https://docs.unity3d.com/Manual/CustomizingYourWorkspace.html)

## 常见问题

### Q: 为什么还需要创建主窗口？
A: ImGui 的多视口功能需要一个"主视口"（Primary Viewport）作为所有其他窗口的根容器。这个主窗口承载了 D3D11 的 SwapChain，用于最终的屏幕输出。子窗口可以拖出成独立窗口，ImGui 会自动管理。

### Q: 如何处理视口与全屏游戏的切换？
A: 编辑器模式下使用 DockSpace + Viewport 窗口；运行游戏时，隐藏所有 ImGui 窗口，直接渲染到 SwapChain。可以用一个 `m_editor_mode` 标志位控制。

### Q: 多视口对性能有影响吗？
A: 有轻微影响，因为每个独立窗口都需要单独的渲染通道。但对于编辑器来说，多显示器支持带来的便利性远大于性能损失。运行游戏时禁用多视口即可。

### Q: 如何保存窗口布局？
A: ImGui 会自动保存布局到 `imgui.ini` 文件（当前在项目根目录）。可以通过 `io.IniFilename` 自定义路径。

## 下一步计划

1. **立即任务**：实现离屏渲染纹理，让 3D 场景显示在 Viewport 中
2. **短期任务**：添加视口交互（相机控制、对象拾取）
3. **中期任务**：添加更多编辑器窗口（场景树、属性面板等）
4. **长期任务**：完整的编辑器功能（Gizmo、撤销重做、资源管理等）

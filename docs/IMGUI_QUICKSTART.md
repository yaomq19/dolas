# ImGui 快速入门指南

## 什么是 ImGui？

**Dear ImGui** 是一个轻量级的即时模式图形用户界面（Immediate Mode GUI）库，专为 C++ 设计。

### 核心特点

- **即时模式**：每帧重新构建 UI，无需维护复杂的 UI 状态
- **易于集成**：只需几行代码即可开始使用
- **高效**：优化的渲染性能，适合游戏和实时应用
- **跨平台**：支持 Windows、Linux、macOS 等多平台
- **灵活**：支持多种渲染后端（DX11、DX12、Vulkan、OpenGL 等）

---

## 项目集成步骤

### 1. vcpkg 依赖（已配置）

```json
{
    "name": "imgui",
    "features": ["dx11-binding", "win32-binding"]
}
```

### 2. CMake 配置（已配置）

```cmake
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
```

### 3. 安装依赖

在项目根目录运行：

```powershell
# 重新配置 CMake 以安装 ImGui
cmake --preset=default
```

vcpkg 会自动下载并编译 ImGui。

---

## 基本使用流程

### 初始化 ImGui（一次性设置）

```cpp
#include <imgui.h>
#include <imgui_impl_win32.h>  // Win32 后端
#include <imgui_impl_dx11.h>   // Direct3D 11 后端

// 在初始化代码中（例如构造函数或 Init 函数）
void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    // 1. 创建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // 可选：启用键盘导航
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // 可选：启用停靠和多视口
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // 2. 设置 ImGui 样式
    ImGui::StyleColorsDark();  // 或 ImGui::StyleColorsLight();
    
    // 3. 初始化平台/渲染后端
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}
```

### 每帧渲染流程

```cpp
void RenderImGui()
{
    // 1. 开始新帧
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // 2. 构建 UI（这里是你的 UI 代码）
    {
        ImGui::Begin("Demo Window");
        ImGui::Text("Hello, ImGui!");
        
        static float f = 0.0f;
        ImGui::SliderFloat("Float", &f, 0.0f, 1.0f);
        
        if (ImGui::Button("Click Me"))
        {
            // 按钮点击逻辑
        }
        
        ImGui::End();
    }
    
    // 3. 渲染
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
```

### 清理资源

```cpp
void ShutdownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
```

### Win32 消息处理

在你的 `WndProc` 函数中添加：

```cpp
// 在 WndProc 函数的开头添加
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 让 ImGui 处理消息
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;
    
    // 你的窗口消息处理...
    switch (msg)
    {
        // ...
    }
}
```

---

## 常用控件示例

### 文本显示

```cpp
ImGui::Text("Simple text");
ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Red text");
ImGui::TextWrapped("This is a long text that will wrap...");
```

### 按钮

```cpp
if (ImGui::Button("Normal Button"))
{
    // 点击处理
}

if (ImGui::SmallButton("Small"))
{
    // 小按钮
}
```

### 输入控件

```cpp
static char text[256] = "";
ImGui::InputText("Text Input", text, IM_ARRAYSIZE(text));

static float value = 0.0f;
ImGui::InputFloat("Float Input", &value);

static int counter = 0;
ImGui::InputInt("Int Input", &counter);
```

### 滑块

```cpp
static float f = 0.5f;
ImGui::SliderFloat("Float Slider", &f, 0.0f, 1.0f);

static int i = 50;
ImGui::SliderInt("Int Slider", &i, 0, 100);

static float vec3[3] = {0.0f, 0.0f, 0.0f};
ImGui::SliderFloat3("Position", vec3, -10.0f, 10.0f);
```

### 复选框和单选按钮

```cpp
static bool checked = false;
ImGui::Checkbox("Enable Feature", &checked);

static int selected = 0;
ImGui::RadioButton("Option 1", &selected, 0);
ImGui::RadioButton("Option 2", &selected, 1);
ImGui::RadioButton("Option 3", &selected, 2);
```

### 组合框（下拉菜单）

```cpp
static int current_item = 0;
const char* items[] = { "Item 1", "Item 2", "Item 3" };
ImGui::Combo("Combo", &current_item, items, IM_ARRAYSIZE(items));
```

### 颜色选择器

```cpp
static float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
ImGui::ColorEdit4("Color", color);
ImGui::ColorPicker4("Color Picker", color);
```

### 树节点

```cpp
if (ImGui::TreeNode("Tree Node"))
{
    ImGui::Text("Content inside tree");
    ImGui::TreePop();
}
```

### 分组和布局

```cpp
// 水平布局
ImGui::SameLine();

// 缩进
ImGui::Indent();
ImGui::Unindent();

// 分隔线
ImGui::Separator();

// 间距
ImGui::Spacing();
```

---

## 完整示例：调试面板

```cpp
void RenderDebugPanel()
{
    // 创建窗口
    ImGui::Begin("Debug Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    // FPS 计数器
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    
    // 相机控制
    if (ImGui::CollapsingHeader("Camera"))
    {
        static float position[3] = {0.0f, 0.0f, 5.0f};
        ImGui::SliderFloat3("Position", position, -10.0f, 10.0f);
        
        static float fov = 45.0f;
        ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f);
    }
    
    // 渲染设置
    if (ImGui::CollapsingHeader("Rendering"))
    {
        static bool wireframe = false;
        ImGui::Checkbox("Wireframe Mode", &wireframe);
        
        static float ambient[3] = {0.1f, 0.1f, 0.1f};
        ImGui::ColorEdit3("Ambient Color", ambient);
    }
    
    // 性能统计
    if (ImGui::CollapsingHeader("Performance"))
    {
        ImGui::Text("Draw Calls: %d", g_DrawCalls);
        ImGui::Text("Triangles: %d", g_TriangleCount);
        ImGui::Text("Memory: %.2f MB", g_MemoryUsage / 1024.0f / 1024.0f);
    }
    
    ImGui::End();
}
```

---

## 常见使用技巧

### 1. 窗口标志

```cpp
ImGuiWindowFlags flags = 0;
flags |= ImGuiWindowFlags_NoTitleBar;      // 无标题栏
flags |= ImGuiWindowFlags_NoResize;        // 不可调整大小
flags |= ImGuiWindowFlags_NoMove;          // 不可移动
flags |= ImGuiWindowFlags_NoCollapse;      // 不可折叠
flags |= ImGuiWindowFlags_AlwaysAutoResize; // 自动调整大小

ImGui::Begin("Window", nullptr, flags);
```

### 2. 样式定制

```cpp
ImGuiStyle& style = ImGui::GetStyle();
style.WindowRounding = 5.0f;
style.FrameRounding = 3.0f;
style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
```

### 3. 字体加载

```cpp
ImGuiIO& io = ImGui::GetIO();
io.Fonts->AddFontFromFileTTF("path/to/font.ttf", 16.0f);
```

### 4. 状态保存

```cpp
// ImGui 会自动保存窗口位置和大小到 imgui.ini 文件
// 可以禁用：
io.IniFilename = nullptr;
```

---

## 调试技巧

### 显示 Demo 窗口

ImGui 自带一个功能完整的演示窗口，展示所有控件：

```cpp
ImGui::ShowDemoWindow();
```

### 显示样式编辑器

```cpp
ImGui::Begin("Style Editor");
ImGui::ShowStyleEditor();
ImGui::End();
```

### 显示度量信息

```cpp
ImGui::ShowMetricsWindow();
```

---

## 性能注意事项

1. **避免每帧创建字符串**：使用静态缓冲区或缓存字符串
2. **使用 ID 栈**：当有重复标签时使用 `PushID/PopID`
3. **条件渲染**：使用 `CollapsingHeader` 或窗口折叠减少不必要的控件
4. **批量操作**：尽量在一个 Begin/End 块中完成相关 UI

---

## 进阶功能

### 自定义渲染

```cpp
ImDrawList* draw_list = ImGui::GetWindowDrawList();
draw_list->AddLine(ImVec2(0, 0), ImVec2(100, 100), IM_COL32(255, 0, 0, 255));
draw_list->AddCircle(ImVec2(50, 50), 30, IM_COL32(0, 255, 0, 255));
```

### 表格布局

```cpp
if (ImGui::BeginTable("table", 3))
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn(); ImGui::Text("Row 1, Col 1");
    ImGui::TableNextColumn(); ImGui::Text("Row 1, Col 2");
    ImGui::TableNextColumn(); ImGui::Text("Row 1, Col 3");
    ImGui::EndTable();
}
```

---

## 参考资源

- **官方仓库**：https://github.com/ocornut/imgui
- **官方 Wiki**：https://github.com/ocornut/imgui/wiki
- **FAQ**：https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
- **示例代码**：`imgui/examples/` 目录

---

## 项目集成检查清单

- [x] 添加 vcpkg 依赖
- [x] 配置 CMakeLists.txt
- [ ] 运行 `cmake --preset=default` 安装依赖
- [ ] 在引擎初始化代码中调用 `ImGui::CreateContext()`
- [ ] 在引擎初始化代码中调用 `ImGui_ImplWin32_Init()` 和 `ImGui_ImplDX11_Init()`
- [ ] 在渲染循环中调用 `ImGui::NewFrame()` → 构建 UI → `ImGui::Render()`
- [ ] 在 WndProc 中转发消息给 ImGui
- [ ] 在引擎关闭时调用清理函数

---

## 快速测试代码

在你的渲染函数中添加以下代码快速测试：

```cpp
// 开始 ImGui 帧
ImGui_ImplDX11_NewFrame();
ImGui_ImplWin32_NewFrame();
ImGui::NewFrame();

// 显示 Demo 窗口
ImGui::ShowDemoWindow();

// 渲染
ImGui::Render();
ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
```

如果能看到 Demo 窗口，说明集成成功！


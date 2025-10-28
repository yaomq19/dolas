// ImGui 集成示例代码
// 这个文件展示如何在 Dolas 引擎中集成 ImGui

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <windows.h>

// ============================================
// 1. 全局变量（通常在你的引擎类中作为成员变量）
// ============================================
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
HWND g_hwnd = nullptr;

// ============================================
// 2. ImGui 初始化函数
// ============================================
void InitImGui()
{
    // 创建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 配置标志
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 启用键盘导航
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // 启用停靠（可选）

    // 设置样式
    ImGui::StyleColorsDark();
    // 或者使用亮色主题：ImGui::StyleColorsLight();

    // 自定义样式（可选）
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    
    // 初始化平台/渲染后端
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}

// ============================================
// 3. ImGui 清理函数
// ============================================
void ShutdownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// ============================================
// 4. 每帧渲染 ImGui
// ============================================
void RenderImGui()
{
    // 开始新帧
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ========== 你的 UI 代码开始 ==========

    // 示例 1: 简单窗口
    {
        ImGui::Begin("Hello, Dolas!");
        ImGui::Text("This is a simple ImGui window");
        
        static float f = 0.0f;
        static int counter = 0;
        
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&g_ClearColor);
        
        if (ImGui::Button("Button"))
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, 
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 示例 2: 性能监控面板
    {
        ImGui::Begin("Performance Monitor");
        
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Separator();
        
        // 绘制 FPS 曲线
        static float fps_values[90] = {};
        static int fps_values_offset = 0;
        fps_values[fps_values_offset] = ImGui::GetIO().Framerate;
        fps_values_offset = (fps_values_offset + 1) % IM_ARRAYSIZE(fps_values);
        
        ImGui::PlotLines("FPS History", fps_values, IM_ARRAYSIZE(fps_values), 
                         fps_values_offset, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
        
        ImGui::End();
    }

    // 示例 3: 场景控制面板
    {
        ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static float camera_pos[3] = {0.0f, 0.0f, 5.0f};
            static float camera_rot[3] = {0.0f, 0.0f, 0.0f};
            static float fov = 45.0f;
            
            ImGui::DragFloat3("Position", camera_pos, 0.1f);
            ImGui::DragFloat3("Rotation", camera_rot, 1.0f, -180.0f, 180.0f);
            ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f);
            
            if (ImGui::Button("Reset Camera"))
            {
                camera_pos[0] = camera_pos[1] = 0.0f;
                camera_pos[2] = 5.0f;
                camera_rot[0] = camera_rot[1] = camera_rot[2] = 0.0f;
                fov = 45.0f;
            }
        }
        
        if (ImGui::CollapsingHeader("Lighting"))
        {
            static bool enable_shadows = true;
            static float ambient_strength = 0.1f;
            static float light_color[3] = {1.0f, 1.0f, 1.0f};
            static float light_dir[3] = {-1.0f, -1.0f, -1.0f};
            
            ImGui::Checkbox("Enable Shadows", &enable_shadows);
            ImGui::SliderFloat("Ambient Strength", &ambient_strength, 0.0f, 1.0f);
            ImGui::ColorEdit3("Light Color", light_color);
            ImGui::DragFloat3("Light Direction", light_dir, 0.01f);
        }
        
        if (ImGui::CollapsingHeader("Rendering"))
        {
            static bool wireframe = false;
            static bool show_normals = false;
            static int msaa_samples = 4;
            
            ImGui::Checkbox("Wireframe Mode", &wireframe);
            ImGui::Checkbox("Show Normals", &show_normals);
            ImGui::SliderInt("MSAA Samples", &msaa_samples, 1, 8);
        }
        
        ImGui::End();
    }

    // 示例 4: 资源浏览器
    {
        ImGui::Begin("Resource Browser");
        
        if (ImGui::BeginTabBar("ResourceTabs"))
        {
            if (ImGui::BeginTabItem("Meshes"))
            {
                static int selected_mesh = 0;
                const char* meshes[] = {"cube.mesh", "plane.mesh", "sphere.mesh", "backpack.obj"};
                ImGui::ListBox("##meshes", &selected_mesh, meshes, IM_ARRAYSIZE(meshes), 5);
                
                if (ImGui::Button("Load Selected"))
                {
                    // 加载选中的网格
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Materials"))
            {
                static int selected_material = 0;
                const char* materials[] = {"solid.material", "deferred_shading.material", "sky_box.material"};
                ImGui::ListBox("##materials", &selected_material, materials, IM_ARRAYSIZE(materials), 5);
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Textures"))
            {
                ImGui::Text("Texture preview coming soon...");
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::End();
    }

    // 示例 5: 控制台日志窗口
    {
        static ImGuiTextBuffer log_buffer;
        static bool auto_scroll = true;
        
        ImGui::Begin("Console");
        
        if (ImGui::Button("Clear"))
            log_buffer.clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &auto_scroll);
        
        ImGui::Separator();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        ImGui::TextUnformatted(log_buffer.begin(), log_buffer.end());
        
        if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        
        ImGui::EndChild();
        ImGui::End();
    }

    // 可选：显示 ImGui 自带的演示窗口（用于学习和调试）
    static bool show_demo_window = true;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // ========== 你的 UI 代码结束 ==========

    // 渲染 ImGui
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// ============================================
// 5. Windows 消息处理
// ============================================
// 在你的 WndProc 函数中添加以下代码

// 在文件开头声明外部函数
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 让 ImGui 先处理消息
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    // 你的消息处理代码
    switch (msg)
    {
    case WM_SIZE:
        // 处理窗口大小变化
        break;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    // 其他消息处理...
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================
// 6. 主循环集成示例
// ============================================
void MainLoop()
{
    // 消息循环
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // 处理 Windows 消息
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // ===== 渲染帧 =====
        
        // 1. 清除渲染目标
        float clear_color[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        g_pd3dDeviceContext->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        
        // 2. 渲染你的 3D 场景
        // RenderScene();
        
        // 3. 渲染 ImGui UI（在场景之后渲染，显示在最上层）
        RenderImGui();
        
        // 4. 呈现到屏幕
        g_pSwapChain->Present(1, 0);  // VSync
    }
}

// ============================================
// 7. 完整的引擎类集成示例
// ============================================
class DolasEngine
{
public:
    void Initialize()
    {
        // 初始化 D3D11...
        
        // 初始化 ImGui（在 D3D11 之后）
        InitImGui();
    }
    
    void Run()
    {
        MainLoop();
    }
    
    void Shutdown()
    {
        // 清理 ImGui（在 D3D11 之前）
        ShutdownImGui();
        
        // 清理 D3D11...
    }
    
private:
    void Update(float deltaTime)
    {
        // 更新游戏逻辑
    }
    
    void Render()
    {
        // 开始帧
        BeginFrame();
        
        // 渲染 3D 场景
        RenderScene();
        
        // 渲染 ImGui UI
        RenderImGui();
        
        // 结束帧
        EndFrame();
    }
};

// ============================================
// 使用提示
// ============================================
/*

编译步骤：
1. 确保已运行 `cmake --preset=default`
2. 重新编译项目

集成检查：
✓ 包含头文件：imgui.h, imgui_impl_win32.h, imgui_impl_dx11.h
✓ 在初始化时调用 InitImGui()
✓ 在主循环中调用 RenderImGui()
✓ 在 WndProc 中转发消息给 ImGui
✓ 在退出时调用 ShutdownImGui()

常见问题：
Q: ImGui 窗口不显示？
A: 检查是否在渲染场景之后调用 ImGui::Render()

Q: 鼠标/键盘输入不响应？
A: 检查 WndProc 中是否正确转发消息给 ImGui_ImplWin32_WndProcHandler

Q: 字体模糊？
A: 考虑加载高分辨率字体或使用 DPI 缩放

Q: 性能问题？
A: 避免在 ImGui 回调中做复杂计算，使用 CollapsingHeader 折叠不需要的面板

*/


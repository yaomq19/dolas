#include <imgui.h>
#include <imgui_impl_win32.h>  // Win32 后端
#include <imgui_impl_dx11.h>   // Direct3D 11 后端
#include <string>
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_imgui_manager.h"
#include "manager/dolas_input_manager.h"
#include "manager/dolas_render_camera_manager.h"
#include "manager/dolas_timer_manager.h"
#include "manager/dolas_shader_manager.h"
#include "manager/dolas_render_pipeline_manager.h"
namespace Dolas
{
	struct FontConfig
	{
		std::string font_file_path;
		Float font_size;
	} k_font_configs[static_cast<UInt>(FontStyle::FontStyleCount)];

    ImGuiManager::ImGuiManager()
        : m_is_imgui_window_open(false)
        , m_viewport_hovered(false)
        , m_viewport_focused(false)
        , m_viewport_size(0.0f, 0.0f)
        , m_viewport_pos(0.0f, 0.0f)
    {
    }

    ImGuiManager::~ImGuiManager()
    {
    }

    bool ImGuiManager::Initialize()
    {
        // 1. 创建 ImGui 上下文
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
		k_font_configs[static_cast<UInt>(FontStyle::LargeFont)] = { "C:/Windows/Fonts/arial.ttf", 24.0f };
		k_font_configs[static_cast<UInt>(FontStyle::SmallFont)] = { "C:/Windows/Fonts/arial.ttf", 12.0f };
		k_font_configs[static_cast<UInt>(FontStyle::BoldFont)] = { "C:/Windows/Fonts/arialbd.ttf", 16.0f };

        // 加载不同大小的字体
        ImFont* font_default = io.Fonts->AddFontDefault();
        for (UInt style = 1; style < static_cast<UInt>(FontStyle::FontStyleCount); style++)
        {
            ImFont* font_large = io.Fonts->AddFontFromFileTTF(k_font_configs[style].font_file_path.c_str(), k_font_configs[style].font_size);
        }

        
        
        // 可选：加载中文字体
        // ImFont* font_chinese = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

        // 启用键盘导航
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // 启用停靠和多视口（方案C核心配置）
        // 注意：需要 ImGui docking 分支支持，vcpkg 默认版本不支持
        // 如果编译出错，请参考 docs/IMGUI_VIEWPORT_INTEGRATION.md
#ifdef IMGUI_HAS_DOCK
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
        // 当启用 ViewportsEnable 时，调整窗口圆角以适应平台窗口
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
#endif
        
        // 2. 设置 ImGui 样式
        ImGui::StyleColorsDark();  // 或 ImGui::StyleColorsLight();
        
        // 3. 初始化平台/渲染后端
        ImGui_ImplWin32_Init(g_dolas_engine.m_rhi->GetWindowHandle());
        ImGui_ImplDX11_Init(g_dolas_engine.m_rhi->GetD3D11Device(), g_dolas_engine.m_rhi->GetD3D11DeviceContext());
    
        return true;
    }

    bool ImGuiManager::Clear()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return true;
    }

    void ImGuiManager::Render()
    {
        // 1. 开始新帧
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // 2. 渲染主 DockSpace（占据整个主窗口）
        RenderMainDockSpace();
        
        // 3. 渲染 3D 视口窗口（可停靠）
        RenderViewportWindow();
        
        // 4. 渲染调试工具窗口（F11 切换）
        if (m_is_imgui_window_open)
        {
            RenderDebugToolsWindow();
        }
        
        // 5. 渲染
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        // 6. 更新和渲染额外的平台窗口（多视口支持）
#ifdef IMGUI_HAS_VIEWPORT
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
#endif
    }

    void ImGuiManager::Tick()
    {
        EventOccurState f11_event_occur_state = g_dolas_engine.m_input_manager->ConsumeKeyEvent(VK_F11);
        switch (f11_event_occur_state)
        {
        case Dolas::EventOccurState::PRESS:
			m_is_imgui_window_open = !m_is_imgui_window_open;
            break;
        case Dolas::EventOccurState::RELEASE:
            break;
        case Dolas::EventOccurState::NONE:
            break;
        default:
            break;
        }
    }

    void ImGuiManager::SetFontStyle(FontStyle font_style)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(io.Fonts->Fonts[static_cast<UInt>(font_style)]);
    }

	void ImGuiManager::UnsetFontStyle()
	{
        ImGuiIO& io = ImGui::GetIO();
		ImGui::PopFont();
	}

    void ImGuiManager::RenderMainDockSpace()
    {
#ifdef IMGUI_HAS_DOCK
        // 创建一个覆盖整个主窗口的 DockSpace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace Main", nullptr, window_flags);
        ImGui::PopStyleVar(3);
        
        // 创建 DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        
        // 顶部菜单栏（可选）
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit")) 
                {
                    PostQuitMessage(0);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Debug Tools (F11)", nullptr, &m_is_imgui_window_open);
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About")) 
                {
                    // TODO: 显示关于对话框
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
        
        ImGui::End();
#else
        // 如果没有 docking 支持，显示简单的菜单栏
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit")) 
                {
                    PostQuitMessage(0);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Debug Tools (F11)", nullptr, &m_is_imgui_window_open);
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About")) 
                {
                    // TODO: 显示关于对话框
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMainMenuBar();
        }
#endif
    }
    
    void ImGuiManager::RenderViewportWindow()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("3D Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        // 获取可用区域
        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        m_viewport_size = viewport_panel_size;
        m_viewport_pos = ImGui::GetCursorScreenPos();
        
        // 检查视口状态
        m_viewport_hovered = ImGui::IsWindowHovered();
        m_viewport_focused = ImGui::IsWindowFocused();
        
        // TODO: 这里将来要显示渲染到离屏纹理的 3D 场景
        // 目前先显示一个占位符
        ImGui::Text("3D Viewport");
        ImGui::Text("Size: %.0f x %.0f", viewport_panel_size.x, viewport_panel_size.y);
        ImGui::Text("Hovered: %s", m_viewport_hovered ? "Yes" : "No");
        ImGui::Text("Focused: %s", m_viewport_focused ? "Yes" : "No");
        
        // 示例：绘制一个网格背景
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = m_viewport_pos;
        ImVec2 canvas_sz = viewport_panel_size;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // 绘制背景
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        
        // 绘制网格
        const float grid_step = 64.0f;
        for (float x = fmodf(0.0f, grid_step); x < canvas_sz.x; x += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
        for (float y = fmodf(0.0f, grid_step); y < canvas_sz.y; y += grid_step)
            draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
        
        // 当鼠标在视口内时，可以捕获鼠标输入
        if (m_viewport_hovered)
        {
            ImGuiIO& io = ImGui::GetIO();
            
            // 示例：显示鼠标位置（视口局部坐标）
            ImVec2 mouse_pos_in_canvas = ImVec2(io.MousePos.x - canvas_p0.x, io.MousePos.y - canvas_p0.y);
            if (mouse_pos_in_canvas.x >= 0 && mouse_pos_in_canvas.y >= 0 && 
                mouse_pos_in_canvas.x < canvas_sz.x && mouse_pos_in_canvas.y < canvas_sz.y)
            {
                // 绘制鼠标位置指示器
                draw_list->AddCircleFilled(io.MousePos, 5.0f, IM_COL32(255, 255, 0, 200));
                
                // 显示坐标
                char buf[64];
                sprintf_s(buf, "Mouse: (%.0f, %.0f)", mouse_pos_in_canvas.x, mouse_pos_in_canvas.y);
                draw_list->AddText(ImVec2(canvas_p0.x + 10, canvas_p0.y + 10), IM_COL32(255, 255, 255, 255), buf);
            }
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    void ImGuiManager::RenderDebugToolsWindow()
    {
        SetFontStyle(FontStyle::BoldFont);

        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Debug Tools", &m_is_imgui_window_open);
        
        if (ImGui::Button("Dump Camera Info"))
        {
            g_dolas_engine.m_render_camera_manager->DumpCameraInfo();
        }
        
        if (ImGui::Button("Dump Shader Info"))
        {
            g_dolas_engine.m_shader_manager->dumpShaderReflectionInfos();
        }

        if (ImGui::Button("Display World Coordinate System"))
        {
            g_dolas_engine.m_render_pipeline_manager->DisplayWorldCoordinateSystem();
        }

        ImGui::Separator();
        
        // FPS 信息显示
        static Float fps = 0.0f;
        
        if (g_dolas_engine.m_timer_manager->GetFrameCount() % 50 == 0)
        {
            fps = g_dolas_engine.m_timer_manager->GetFPS();
        }
        ImGui::Text("FPS: %.2f", fps);
        
        ImGui::Separator();
        
        // 视口信息
        ImGui::Text("Viewport Size: %.0f x %.0f", m_viewport_size.x, m_viewport_size.y);
        ImGui::Text("Viewport Hovered: %s", m_viewport_hovered ? "Yes" : "No");
        ImGui::Text("Viewport Focused: %s", m_viewport_focused ? "Yes" : "No");

        UnsetFontStyle();
        ImGui::End();
    }
}
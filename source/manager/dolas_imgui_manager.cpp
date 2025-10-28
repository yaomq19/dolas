#include <imgui.h>
#include <imgui_impl_win32.h>  // Win32 后端
#include <imgui_impl_dx11.h>   // Direct3D 11 后端
#include "core/dolas_engine.h"
#include "core/dolas_rhi.h"
#include "manager/dolas_imgui_manager.h"
#include "manager/dolas_input_manager.h"
#include "manager/dolas_render_camera_manager.h"
namespace Dolas
{
    ImGuiManager::ImGuiManager()
    {
        m_is_imgui_window_open = false;
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
        
        // 可选：启用键盘导航
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // 可选：启用停靠和多视口
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
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
        
        // 2. 构建 UI（这里是你的 UI 代码）
        if (m_is_imgui_window_open)
        {
            ImGui::Begin("ImGUI Window");
            /*ImGui::Text("Hello, ImGui!");
            
			static float f = 0.0f;
			ImGui::SliderFloat("Float", &f, 0.0f, 1.0f);*/
            
            if (ImGui::Button("Dump Camera Info"))
            {
                g_dolas_engine.m_render_camera_manager->DumpCameraInfo();
            }
            
            ImGui::End();
        }
        
        // 3. 渲染
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
}
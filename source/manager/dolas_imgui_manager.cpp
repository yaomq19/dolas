#include <imgui.h>
#include <imgui_internal.h> // DockBuilder API (docking 分支)
#include <imgui_impl_win32.h>  // Win32 后端
#include <imgui_impl_dx11.h>   // Direct3D 11 后端
#include <string>
#include <algorithm>  // for std::max
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
        , m_dockspace_initialized(false)
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
        
        // 启用 Docking（需要 imgui 的 docking 分支）
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // 注意：这里先不启用 ViewportsEnable（多窗口），避免面板被弹成独立系统窗口
        
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
        
        // 3. 渲染固定的编辑器窗口（不可关闭）
        RenderSceneHierarchyWindow();
        RenderPropertiesWindow();
        RenderContentBrowserWindow();
        
        // 4. 渲染调试工具窗口（F11 切换）
        if (m_is_imgui_window_open)
        {
            RenderDebugToolsWindow();
        }
        
        // 5. 渲染
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
        // 创建一个覆盖整个主窗口的 DockSpace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        // 让 DockSpace Host 背景透明，中央区域透出 3D 渲染
        ImGui::SetNextWindowBgAlpha(0.0f);

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
        
        // 首次运行时初始化布局
        if (!m_dockspace_initialized)
        {
            m_dockspace_initialized = true;
            
            // 清除之前的布局
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);
            
            // UE 风格固定布局：
            // 1) 底部 Content Browser 横跨全宽
            // 2) 上半部分左侧 Scene Hierarchy
            // 3) 上半部分右侧 Properties
            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_bottom = 0;
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_right = 0;
            
            // 步骤1: 从整个 DockSpace 底部切出 25% 给 Content Browser
            dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
            
            // 步骤2: 从上半部分左侧切出 20% 给 Scene Hierarchy  
            dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
            
            // 步骤3: 从剩余部分右侧切出约 20% 给 Properties (0.2/0.8 = 0.25)
            dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
            
            // 设置节点为不可调整大小，无标签栏
            ImGuiDockNode* node_left = ImGui::DockBuilderGetNode(dock_id_left);
            ImGuiDockNode* node_right = ImGui::DockBuilderGetNode(dock_id_right);
            ImGuiDockNode* node_bottom = ImGui::DockBuilderGetNode(dock_id_bottom);
            
            if (node_left)
            {
                node_left->LocalFlags |= ImGuiDockNodeFlags_NoTabBar
                    | ImGuiDockNodeFlags_NoResize
                    | ImGuiDockNodeFlags_NoWindowMenuButton
                    | ImGuiDockNodeFlags_NoDockingOverMe
                    | ImGuiDockNodeFlags_NoDockingSplit;
            }
            if (node_right)
            {
                node_right->LocalFlags |= ImGuiDockNodeFlags_NoTabBar
                    | ImGuiDockNodeFlags_NoResize
                    | ImGuiDockNodeFlags_NoWindowMenuButton
                    | ImGuiDockNodeFlags_NoDockingOverMe
                    | ImGuiDockNodeFlags_NoDockingSplit;
            }
            if (node_bottom)
            {
                node_bottom->LocalFlags |= ImGuiDockNodeFlags_NoTabBar
                    | ImGuiDockNodeFlags_NoResize
                    | ImGuiDockNodeFlags_NoWindowMenuButton
                    | ImGuiDockNodeFlags_NoDockingOverMe
                    | ImGuiDockNodeFlags_NoDockingSplit;
            }
            
            // 停靠窗口到对应节点
            ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Properties", dock_id_right);
            ImGui::DockBuilderDockWindow("Content Browser", dock_id_bottom);
            
            ImGui::DockBuilderFinish(dockspace_id);
        }
        
        // 让中央区域不画背景、鼠标可穿透（便于渲染画面作为主背景）
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        
        // 顶部菜单栏
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
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout"))
                {
                    m_dockspace_initialized = false;
                }
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
    
    void ImGuiManager::RenderSceneHierarchyWindow()
    {
        // 强制窗口填充整个停靠区域，无标题栏
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        
        ImGui::Begin("Scene Hierarchy", nullptr, window_flags);
        
        ImGui::Text("SCENE HIERARCHY");
        ImGui::Separator();
        
        // 示例：显示场景对象树
        if (ImGui::TreeNode("Root Scene"))
        {
            // TODO: 从场景管理器获取实际的场景对象
            if (ImGui::TreeNode("Main Camera"))
            {
                ImGui::Text("Type: Camera");
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Directional Light"))
            {
                ImGui::Text("Type: Light");
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Entities"))
            {
                // 示例实体
                if (ImGui::Selectable("Ground Plane"))
                {
                    // TODO: 选中该实体
                }
                if (ImGui::Selectable("Sphere"))
                {
                    // TODO: 选中该实体
                }
                if (ImGui::Selectable("Hammer"))
                {
                    // TODO: 选中该实体
                }
                ImGui::TreePop();
            }
            
            ImGui::TreePop();
        }
        
        ImGui::Separator();
        
        // 添加/删除对象按钮
        if (ImGui::Button("Add Entity"))
        {
            // TODO: 添加新实体
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected"))
        {
            // TODO: 删除选中的实体
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    void ImGuiManager::RenderPropertiesWindow()
    {
        // 强制窗口填充整个停靠区域，无标题栏
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        
        ImGui::Begin("Properties", nullptr, window_flags);
        
        ImGui::Text("PROPERTIES");
        ImGui::Separator();
        
        // 示例：显示选中对象的属性
        static char name[128] = "Selected Object";
        ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
        
        ImGui::Spacing();
        
        // Transform 组件
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static float position[3] = { 0.0f, 0.0f, 0.0f };
            static float rotation[3] = { 0.0f, 0.0f, 0.0f };
            static float scale[3] = { 1.0f, 1.0f, 1.0f };
            
            ImGui::DragFloat3("Position", position, 0.1f);
            ImGui::DragFloat3("Rotation", rotation, 1.0f);
            ImGui::DragFloat3("Scale", scale, 0.1f);
        }
        
        // Material 组件
        if (ImGui::CollapsingHeader("Material"))
        {
            const char* materials[] = { "Default", "PBR", "Unlit", "Custom" };
            static int current_material = 0;
            ImGui::Combo("Material Type", &current_material, materials, IM_ARRAYSIZE(materials));
            
            static float albedo[3] = { 1.0f, 1.0f, 1.0f };
            ImGui::ColorEdit3("Albedo", albedo);
            
            static float metallic = 0.0f;
            ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
            
            static float roughness = 0.5f;
            ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
        }
        
        // Mesh 组件
        if (ImGui::CollapsingHeader("Mesh Renderer"))
        {
            ImGui::Text("Mesh: cube.mesh");
            if (ImGui::Button("Select Mesh..."))
            {
                // TODO: 打开资源选择器
            }
            
            ImGui::Checkbox("Cast Shadows", nullptr);
            ImGui::Checkbox("Receive Shadows", nullptr);
        }
        
        ImGui::Separator();
        
        // 添加组件按钮
        if (ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("add_component_popup");
        }
        
        if (ImGui::BeginPopup("add_component_popup"))
        {
            ImGui::Text("Components");
            ImGui::Separator();
            if (ImGui::Selectable("Mesh Renderer")) {}
            if (ImGui::Selectable("Light")) {}
            if (ImGui::Selectable("Camera")) {}
            if (ImGui::Selectable("Script")) {}
            ImGui::EndPopup();
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
    }
    
    void ImGuiManager::RenderContentBrowserWindow()
    {
        // 强制窗口填充整个停靠区域，无标题栏
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        
        ImGui::Begin("Content Browser", nullptr, window_flags);
        
        ImGui::Text("CONTENT BROWSER");
        ImGui::Separator();
        
        // 目录树
        ImGui::BeginChild("Directories", ImVec2(150, 0), true);
        if (ImGui::TreeNode("content/"))
        {
            if (ImGui::TreeNode("scene/"))
            {
                ImGui::Selectable("default_scene/");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("mesh/"))
            {
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("texture/"))
            {
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("shader/"))
            {
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("material/"))
            {
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // 文件列表
        ImGui::BeginChild("Files", ImVec2(0, 0), true);
        
        // 工具栏
        if (ImGui::Button("Import"))
        {
            // TODO: 导入资源
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh"))
        {
            // TODO: 刷新资源列表
        }
        
        ImGui::Separator();
        
        // 示例：网格视图显示资源
        const float icon_size = 80.0f;
        const float spacing = 10.0f;
        float panel_width = ImGui::GetContentRegionAvail().x;
        int columns = (int)(panel_width / (icon_size + spacing));
        if (columns < 1) columns = 1;
        
        ImGui::Columns(columns, nullptr, false);
        
        // 示例资源图标
        const char* assets[] = {
            "cube.mesh", "sphere.mesh", "plane.mesh",
            "default.material", "pbr.material",
            "texture_001.png", "texture_002.png",
            "shader.hlsl", "scene.scene"
        };
        
        for (int i = 0; i < IM_ARRAYSIZE(assets); i++)
        {
            ImGui::PushID(i);
            
            // 绘制图标占位符
            ImGui::Button(assets[i], ImVec2(icon_size, icon_size));
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                // TODO: 双击打开资源
            }
            
            // 显示文件名
            ImGui::TextWrapped("%s", assets[i]);
            
            ImGui::PopID();
            ImGui::NextColumn();
        }
        
        ImGui::Columns(1);
        ImGui::EndChild();
        
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
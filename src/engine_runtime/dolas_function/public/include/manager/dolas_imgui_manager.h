#ifndef DOLAS_IMGUI_MANAGER_H
#define DOLAS_IMGUI_MANAGER_H

#include "dolas_base.h"
#include <imgui.h>  // 需要完整定义以使用 ImVec2
namespace Dolas
{
    enum class FontStyle : UInt
    {
        DefaultFont,
        LargeFont,
        SmallFont,
        BoldFont,
        FontStyleCount,
    };

    class ImGuiManager
    {
    public:
        ImGuiManager();
        ~ImGuiManager();

        bool Initialize();
        bool Clear();
        void Render();

        void TickPreRender();
        
        // 获取视口信息
        bool IsViewportHovered() const { return m_viewport_hovered; }
        bool IsViewportFocused() const { return m_viewport_focused; }
        ImVec2 GetViewportPos() const { return m_viewport_pos; }     // 相对于主窗口客户区(0,0)
        ImVec2 GetViewportSize() const { return m_viewport_size; }   // 像素尺寸
        
    private:
        void SetFontStyle(FontStyle font_style);
        void UnsetFontStyle();
        
        // 渲染主 DockSpace
        void RenderMainDockSpace();
        
        // 渲染调试工具窗口
        void RenderDebugToolsWindow();
        
        // 渲染场景层级窗口
        void RenderSceneHierarchyWindow();
        
        // 渲染属性面板窗口
        void RenderPropertiesWindow();
        
        // 渲染资源浏览器窗口
        void RenderContentBrowserWindow();
        
        bool m_is_imgui_window_open;
        bool m_dockspace_initialized;  // 用于初始化布局
        ImGuiID m_scene_viewport_node_id; // docking 中心节点 id（用于计算场景视口矩形）
        
        // 视口相关
        bool m_viewport_hovered;
        bool m_viewport_focused;
        ImVec2 m_viewport_size;
        ImVec2 m_viewport_pos;
    };
}// namespace Dolas

#endif // DOLAS_IMGUI_MANAGER_H
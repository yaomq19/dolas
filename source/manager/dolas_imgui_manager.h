#ifndef DOLAS_IMGUI_MANAGER_H
#define DOLAS_IMGUI_MANAGER_H

#include "base/dolas_base.h"
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

        void Tick();
        
        // 获取视口信息
        bool IsViewportHovered() const { return m_viewport_hovered; }
        bool IsViewportFocused() const { return m_viewport_focused; }
        
    private:
        void SetFontStyle(FontStyle font_style);
        void UnsetFontStyle();
        
        // 渲染主 DockSpace
        void RenderMainDockSpace();
        
        // 渲染 3D 视口窗口
        void RenderViewportWindow();
        
        // 渲染调试工具窗口
        void RenderDebugToolsWindow();
        
        bool m_is_imgui_window_open;
        
        // 视口相关
        bool m_viewport_hovered;
        bool m_viewport_focused;
        ImVec2 m_viewport_size;
        ImVec2 m_viewport_pos;
    };
}// namespace Dolas

#endif // DOLAS_IMGUI_MANAGER_H
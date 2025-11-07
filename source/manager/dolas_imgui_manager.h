#ifndef DOLAS_IMGUI_MANAGER_H
#define DOLAS_IMGUI_MANAGER_H

#include "base/dolas_base.h"
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
    private:
        void SetFontStyle(FontStyle font_style);
        void UnsetFontStyle();
        bool m_is_imgui_window_open;
    };
}// namespace Dolas

#endif // DOLAS_IMGUI_MANAGER_H
#ifndef DOLAS_IMGUI_MANAGER_H
#define DOLAS_IMGUI_MANAGER_H

namespace Dolas
{
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
        bool m_is_imgui_window_open;
    };
}// namespace Dolas

#endif // DOLAS_IMGUI_MANAGER_H
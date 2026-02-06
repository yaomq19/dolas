#ifndef DOLAS_INPUT_MANAGER_H
#define DOLAS_INPUT_MANAGER_H

#include <Windows.h>
#include <unordered_map>
#include <queue>
#include "dolas_math.h"

namespace Dolas
{
    enum class KeyState
    {
        UP,
        DOWN
    };

	enum class EventOccurState
	{
		PRESS,
        RELEASE,
        NONE
	};

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();

        bool Initialize();
        void Clear();
        void Tick();

        bool IsKeyDown(int key_code) const;
        bool IsKeyUp(int key_code) const;

        void ProduceKeyPressEvent(int key_code);
        void ProduceKeyReleaseEvent(int key_code);
        EventOccurState ConsumeKeyEvent(int key_code);

        Vector2 GetMouseDelta() const;
        float GetMouseWheelDelta() const;
		bool IsMouseCaptured() const;

        void CaptureMouse(bool capture);
		void ResetMouseWheelDelta() { m_mouse_wheel_delta = 0.0f; }
        LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    private:
        HWND m_window_handle;
        bool m_mouse_captured;

        std::unordered_map<int, KeyState> m_key_states;
        std::unordered_map<int, std::queue<EventOccurState>> m_key_event_occur_states;

        Vector2 m_mouse_position;
        Vector2 m_previous_mouse_position;
        Vector2 m_mouse_delta;
        Vector2 m_screen_center;
        float m_mouse_wheel_delta;
        void UpdateMouseWheelDelta(WPARAM wParam);

        void UpdateKeyState(int key_code, bool is_down);
        void UpdateMousePosition();
    };
} // namespace Dolas

#endif // DOLAS_INPUT_MANAGER_H

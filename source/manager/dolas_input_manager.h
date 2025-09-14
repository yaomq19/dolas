#ifndef DOLAS_INPUT_MANAGER_H
#define DOLAS_INPUT_MANAGER_H

#include <Windows.h>
#include <unordered_map>
#include "core/dolas_math.h"

namespace Dolas
{
    enum class KeyState
    {
        UP,
        DOWN,
        PRESSED,  // 刚按下
        RELEASED  // 刚释放
    };

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();

        bool Initialize(HWND window_handle);
        void Clear();
        void Update();

        // 键盘输入
        bool IsKeyDown(int key_code) const;
        bool IsKeyPressed(int key_code) const;
        bool IsKeyReleased(int key_code) const;

        // 鼠标输入
        bool IsMouseButtonDown(int button) const;
        bool IsMouseButtonPressed(int button) const;
        bool IsMouseButtonReleased(int button) const;
        
        Vector2 GetMousePosition() const;
        Vector2 GetMouseDelta() const;
        
        // 鼠标捕获控制
        void CaptureMouse(bool capture);
        bool IsMouseCaptured() const;

        // 消息处理
        void ProcessKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        void ProcessMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        HWND m_window_handle;
        bool m_mouse_captured;
        
        // 键盘状态
        std::unordered_map<int, KeyState> m_key_states;
        std::unordered_map<int, KeyState> m_previous_key_states;
        
        // 鼠标状态
        std::unordered_map<int, KeyState> m_mouse_button_states;
        std::unordered_map<int, KeyState> m_previous_mouse_button_states;
        
        Vector2 m_mouse_position;
        Vector2 m_previous_mouse_position;
        Vector2 m_mouse_delta;
        
        Vector2 m_screen_center;
        
        void UpdateKeyState(int key_code, bool is_down);
        void UpdateMouseButtonState(int button, bool is_down);
        void UpdateMousePosition();
    };

    // 全局输入管理器实例
    extern InputManager g_input_manager;

} // namespace Dolas

#endif // DOLAS_INPUT_MANAGER_H

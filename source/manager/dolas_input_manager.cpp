#include "dolas_input_manager.h"
#include <iostream>

namespace Dolas
{
    // 全局输入管理器实例
    InputManager::InputManager()
        : m_window_handle(nullptr)
        , m_mouse_captured(false)
        , m_mouse_position(0.0f, 0.0f)
        , m_previous_mouse_position(0.0f, 0.0f)
        , m_mouse_delta(0.0f, 0.0f)
        , m_screen_center(0.0f, 0.0f)
        , m_mouse_wheel_delta(0.0f)
    {
    }

    InputManager::~InputManager()
    {
        Clear();
    }

    bool InputManager::Initialize(HWND window_handle)
    {
        m_window_handle = window_handle;
        
        // 获取窗口中心点
        RECT rect;
        GetClientRect(m_window_handle, &rect);
        m_screen_center.x = (rect.right - rect.left) / 2.0f;
        m_screen_center.y = (rect.bottom - rect.top) / 2.0f;
        
        // 初始化鼠标位置
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        ScreenToClient(m_window_handle, &cursor_pos);
        m_mouse_position.x = static_cast<float>(cursor_pos.x);
        m_mouse_position.y = static_cast<float>(cursor_pos.y);
        m_previous_mouse_position = m_mouse_position;
        
        return true;
    }

    void InputManager::Clear()
    {
        if (m_mouse_captured)
        {
            CaptureMouse(false);
        }
        
        m_key_states.clear();
    }

    void InputManager::Tick()
    {
        // 更新鼠标位置和增量
    }

    bool InputManager::IsKeyDown(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && it->second == KeyState::DOWN;
    }

    Vector2 InputManager::GetMouseDelta() const
    {
        return m_mouse_delta;
    }

    float InputManager::GetMouseWheelDelta() const
    {
        return m_mouse_wheel_delta;
    }

    void InputManager::CaptureMouse(bool capture)
    {
        if (capture && !m_mouse_captured)
        {
            // 捕获鼠标
            SetCapture(m_window_handle);
            
            // 隐藏鼠标光标
            ShowCursor(FALSE);
            
            // 将鼠标移动到屏幕中心
            POINT center_point;
            center_point.x = static_cast<LONG>(m_screen_center.x);
            center_point.y = static_cast<LONG>(m_screen_center.y);
            ClientToScreen(m_window_handle, &center_point);
            SetCursorPos(center_point.x, center_point.y);
            
            m_mouse_captured = true;
        }
        else if (!capture && m_mouse_captured)
        {
            // 释放鼠标捕获
            ReleaseCapture();
            
            // 显示鼠标光标
            ShowCursor(TRUE);
            
            m_mouse_captured = false;
        }
    }

    bool InputManager::IsMouseCaptured() const
    {
        return m_mouse_captured;
    }

    void InputManager::UpdateKeyState(int key_code, bool is_down)
    {
        m_key_states[key_code] = is_down ? KeyState::DOWN : KeyState::UP;
    }

    void InputManager::UpdateMouseWheelDelta(WPARAM wParam)
    {
        DOLAS_RETURN_IF_FALSE(m_mouse_captured);
		// 处理滚轮事件
		// wParam的高16位包含滚轮增量值
		short wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
		// WHEEL_DELTA通常是120，我们将其标准化为1.0或-1.0
		float normalized_delta = static_cast<float>(wheel_delta) / WHEEL_DELTA;
		m_mouse_wheel_delta += normalized_delta;
    }

    void InputManager::UpdateMousePosition()
    {
        DOLAS_RETURN_IF_FALSE(m_mouse_captured);

        m_previous_mouse_position = m_mouse_position;
        // 获取当前鼠标位置
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        ScreenToClient(m_window_handle, &cursor_pos);
            
        // 计算相对于屏幕中心的偏移
        m_mouse_delta.x = cursor_pos.x - m_screen_center.x;
        m_mouse_delta.y = cursor_pos.y - m_screen_center.y;
            
        // 将鼠标重新设置到屏幕中心
        if (m_mouse_delta.x != 0.0f || m_mouse_delta.y != 0.0f)
        {
            POINT center_point;
            center_point.x = static_cast<LONG>(m_screen_center.x);
            center_point.y = static_cast<LONG>(m_screen_center.y);
            ClientToScreen(m_window_handle, &center_point);
            SetCursorPos(center_point.x, center_point.y);
        }
            
        m_mouse_position = m_screen_center;
    }

    LRESULT InputManager::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // 处理输入相关的消息
        switch (msg)
        {
            // Keyboard
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
                UpdateKeyState(static_cast<int>(wParam), true);
				break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
                UpdateKeyState(static_cast<int>(wParam), false);
				break;

            // Mouse
			case WM_RBUTTONDOWN:
				CaptureMouse(true);
				break;

			case WM_RBUTTONUP:
				CaptureMouse(false);
				break;

			case WM_MOUSEWHEEL:
                UpdateMouseWheelDelta(wParam);
				break;

            case WM_MOUSEMOVE:
				UpdateMousePosition();
			default:
                break;
        }

        // 处理系统消息
        switch (msg)
        {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace Dolas

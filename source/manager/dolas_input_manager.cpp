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
        m_previous_key_states.clear();
        m_mouse_button_states.clear();
        m_previous_mouse_button_states.clear();
    }

    void InputManager::Update()
    {
        // 重置滚轮增量（每帧重置）
        // m_mouse_wheel_delta = 0.0f;
        
        // 更新鼠标位置和增量
        UpdateMousePosition();
    }

    bool InputManager::IsKeyDown(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && it->second == KeyState::DOWN;
    }

    bool InputManager::IsMouseButtonDown(int button) const
    {
        auto it = m_mouse_button_states.find(button);
        return it != m_mouse_button_states.end() && it->second == KeyState::DOWN;
    }

    Vector2 InputManager::GetMousePosition() const
    {
        return m_mouse_position;
    }

    Vector2 InputManager::GetMouseDelta() const
    {
        return m_mouse_delta;
    }

    float InputManager::GetMouseWheelDelta() const
    {
        if (m_mouse_wheel_delta != 0.0f)
        {
            std::cerr << "GetMouseWheelDelta() returning: " << m_mouse_wheel_delta << std::endl;
        }
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

    void InputManager::ProcessKeyboardMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        int virtual_key = static_cast<int>(wParam);
        bool is_key_down = false;

		switch (msg)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			is_key_down = true;
			std::cerr << "Key Down: " << virtual_key << " ('" << static_cast<char>(virtual_key) << "')" << std::endl;
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			is_key_down = false;
			std::cerr << "Key Up: " << virtual_key << " ('" << static_cast<char>(virtual_key) << "')" << std::endl;
			break;

		default:
			return; // 不处理其他消息
		}

        UpdateKeyState(virtual_key, is_key_down);
    }

    void InputManager::ProcessMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        int virtual_key = -1;
        bool is_key_down = false;

        switch (msg)
        {
        case WM_LBUTTONDOWN:
            virtual_key = VK_LBUTTON;
            is_key_down = true;
            break;
            
        case WM_LBUTTONUP:
            virtual_key = VK_LBUTTON;
            is_key_down = false;
            break;
            
        case WM_RBUTTONDOWN:
            virtual_key = VK_RBUTTON;
            is_key_down = true;
            CaptureMouse(true);
            break;
            
        case WM_RBUTTONUP:
            virtual_key = VK_RBUTTON;
            is_key_down = false;
			CaptureMouse(false);

            break;
            
        case WM_MBUTTONDOWN:
            virtual_key = VK_MBUTTON;
            is_key_down = true;
            break;
            
        case WM_MBUTTONUP:
            virtual_key = VK_MBUTTON;
            is_key_down = false;
            break;
            
        case WM_MOUSEMOVE:
            virtual_key = -1;
            is_key_down = false;
            // 鼠标移动在UpdateMousePosition中处理
            break;
            
        case WM_MOUSEWHEEL:
            {
                // 处理滚轮事件
                // wParam的高16位包含滚轮增量值
                short wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
                std::cerr << "WM_MOUSEWHEEL received! Raw delta: " << wheel_delta << std::endl;
                
                // 将Windows的滚轮增量转换为标准化的值
                // WHEEL_DELTA通常是120，我们将其标准化为1.0或-1.0
                float normalized_delta = static_cast<float>(wheel_delta) / WHEEL_DELTA;
                m_mouse_wheel_delta += normalized_delta;
                
                std::cerr << "Normalized delta: " << normalized_delta << ", Total delta: " << m_mouse_wheel_delta << std::endl;
                virtual_key = -1; // 滚轮不是按钮事件
            }
            break;
        }

        if (virtual_key != -1)
        {
            UpdateMouseButtonState(virtual_key, is_key_down);
        }
    }

    void InputManager::UpdateKeyState(int key_code, bool is_down)
    {
        if (is_down)
        {
            m_key_states[key_code] = KeyState::DOWN;
        }
        else
        {
            m_key_states[key_code] = KeyState::UP;
        }
    }

    void InputManager::UpdateMouseButtonState(int button, bool is_down)
    {
        if (is_down)
        {
            m_mouse_button_states[button] = KeyState::DOWN;
        }
        else
        {
            m_mouse_button_states[button] = KeyState::UP;
        }

    }

    void InputManager::UpdateMousePosition()
    {
        m_previous_mouse_position = m_mouse_position;
        
        if (m_mouse_captured)
        {
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
        else
        {
            // 正常模式下获取鼠标位置
            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(m_window_handle, &cursor_pos);
            
            m_mouse_position.x = static_cast<float>(cursor_pos.x);
            m_mouse_position.y = static_cast<float>(cursor_pos.y);
            
            m_mouse_delta.x = m_mouse_position.x - m_previous_mouse_position.x;
            m_mouse_delta.y = m_mouse_position.y - m_previous_mouse_position.y;
        }
    }

} // namespace Dolas

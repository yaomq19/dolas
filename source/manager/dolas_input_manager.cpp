#include "dolas_input_manager.h"
#include <iostream>

namespace Dolas
{
    // 全局输入管理器实例
    InputManager g_input_manager;

    InputManager::InputManager()
        : m_window_handle(nullptr)
        , m_mouse_captured(false)
        , m_mouse_position(0.0f, 0.0f)
        , m_previous_mouse_position(0.0f, 0.0f)
        , m_mouse_delta(0.0f, 0.0f)
        , m_screen_center(0.0f, 0.0f)
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
        // 保存上一帧的状态
        m_previous_key_states = m_key_states;
        m_previous_mouse_button_states = m_mouse_button_states;
        
        // 更新键盘状态 - 将PRESSED和RELEASED状态转换为DOWN和UP
        for (auto& pair : m_key_states)
        {
            if (pair.second == KeyState::PRESSED)
            {
                pair.second = KeyState::DOWN;
            }
            else if (pair.second == KeyState::RELEASED)
            {
                pair.second = KeyState::UP;
            }
        }
        
        // 更新鼠标按钮状态
        for (auto& pair : m_mouse_button_states)
        {
            if (pair.second == KeyState::PRESSED)
            {
                pair.second = KeyState::DOWN;
            }
            else if (pair.second == KeyState::RELEASED)
            {
                pair.second = KeyState::UP;
            }
        }
        
        // 更新鼠标位置和增量
        UpdateMousePosition();
    }

    bool InputManager::IsKeyDown(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && (it->second == KeyState::DOWN || it->second == KeyState::PRESSED);
    }

    bool InputManager::IsKeyPressed(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && it->second == KeyState::PRESSED;
    }

    bool InputManager::IsKeyReleased(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && it->second == KeyState::RELEASED;
    }

    bool InputManager::IsMouseButtonDown(int button) const
    {
        auto it = m_mouse_button_states.find(button);
        return it != m_mouse_button_states.end() && (it->second == KeyState::DOWN || it->second == KeyState::PRESSED);
    }

    bool InputManager::IsMouseButtonPressed(int button) const
    {
        auto it = m_mouse_button_states.find(button);
        return it != m_mouse_button_states.end() && it->second == KeyState::PRESSED;
    }

    bool InputManager::IsMouseButtonReleased(int button) const
    {
        auto it = m_mouse_button_states.find(button);
        return it != m_mouse_button_states.end() && it->second == KeyState::RELEASED;
    }

    Vector2 InputManager::GetMousePosition() const
    {
        return m_mouse_position;
    }

    Vector2 InputManager::GetMouseDelta() const
    {
        return m_mouse_delta;
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
        int key_code = static_cast<int>(wParam);
        
        switch (msg)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            UpdateKeyState(key_code, true);
            break;
            
        case WM_KEYUP:
        case WM_SYSKEYUP:
            UpdateKeyState(key_code, false);
            break;
        }
    }

    void InputManager::ProcessMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_LBUTTONDOWN:
			std::cerr << "Left Mouse Button Down" << std::endl;
            UpdateMouseButtonState(VK_LBUTTON, true);
            break;
            
        case WM_LBUTTONUP:
			std::cerr << "Left Mouse Button Up" << std::endl;
            UpdateMouseButtonState(VK_LBUTTON, false);
            break;
            
        case WM_RBUTTONDOWN:
			std::cerr << "Right Mouse Button Down" << std::endl;
            UpdateMouseButtonState(VK_RBUTTON, true);
            break;
            
        case WM_RBUTTONUP:
			std::cerr << "Right Mouse Button Up" << std::endl;
            UpdateMouseButtonState(VK_RBUTTON, false);
            break;
            
        case WM_MBUTTONDOWN:
			std::cerr << "Middle Mouse Button Down" << std::endl;
            UpdateMouseButtonState(VK_MBUTTON, true);
            break;
            
        case WM_MBUTTONUP:
			std::cerr << "Middle Mouse Button Up" << std::endl;
            UpdateMouseButtonState(VK_MBUTTON, false);
            break;
            
        case WM_MOUSEMOVE:
			std::cerr << "Mouse Move" << std::endl;
            // 鼠标移动在UpdateMousePosition中处理
            break;
        }
    }

    void InputManager::UpdateKeyState(int key_code, bool is_down)
    {
        auto current_it = m_key_states.find(key_code);
        auto previous_it = m_previous_key_states.find(key_code);
        
        bool was_down = (previous_it != m_previous_key_states.end()) && 
                       (previous_it->second == KeyState::DOWN || previous_it->second == KeyState::PRESSED);
        
        if (is_down && !was_down)
        {
            m_key_states[key_code] = KeyState::PRESSED;
        }
        else if (!is_down && was_down)
        {
            m_key_states[key_code] = KeyState::RELEASED;
        }
        else if (is_down)
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
        auto previous_it = m_previous_mouse_button_states.find(button);
        
        bool was_down = (previous_it != m_previous_mouse_button_states.end()) && 
                       (previous_it->second == KeyState::DOWN || previous_it->second == KeyState::PRESSED);
        
		// 如果当前帧收到按下事件，且上一帧的状态不是 PRESSED 并且不是 DOWN，则设置为 PRESSED 
        if (is_down && !was_down)
        {
            m_mouse_button_states[button] = KeyState::PRESSED;
        }
        else if (!is_down && was_down)
        {
            m_mouse_button_states[button] = KeyState::RELEASED;
        }
        else if (is_down)
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

#include <iostream>
#include <imgui_impl_win32.h>
#include "dolas_engine.h"
#include "render/dolas_rhi.h"
#include "manager/dolas_input_manager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Dolas
{
    // Global input manager instance
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

    bool InputManager::Initialize()
    {
        m_window_handle = g_dolas_engine.m_rhi->GetWindowHandle();
        
        // Get the window center point
        RECT rect;
        GetClientRect(m_window_handle, &rect);
        m_screen_center.x = (rect.right - rect.left) / 2.0f;
        m_screen_center.y = (rect.bottom - rect.top) / 2.0f;
        
        // Initialize mouse position
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
        // Update mouse position and delta
    }

    bool InputManager::IsKeyDown(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it != m_key_states.end() && it->second == KeyState::DOWN;
    }

    bool InputManager::IsKeyUp(int key_code) const
    {
        auto it = m_key_states.find(key_code);
        return it == m_key_states.end() || it->second == KeyState::UP;
    }

    void InputManager::ProduceKeyPressEvent(int key_code)
    {
        m_key_event_occur_states[key_code].push(EventOccurState::PRESS);
    }

    void InputManager::ProduceKeyReleaseEvent(int key_code)
    {
        m_key_event_occur_states[key_code].push(EventOccurState::RELEASE);
    }

    EventOccurState InputManager::ConsumeKeyEvent(int key_code)
    {
        if (!m_key_event_occur_states[key_code].empty())
        {
            EventOccurState event_occur_state = m_key_event_occur_states[key_code].front();
            m_key_event_occur_states[key_code].pop();
            return event_occur_state;
        }
        return EventOccurState::NONE;
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
            // Capture the mouse
            SetCapture(m_window_handle);
            
            // Hide mouse cursor
            ShowCursor(FALSE);
            
            // Move mouse to screen center
            POINT center_point;
            center_point.x = static_cast<LONG>(m_screen_center.x);
            center_point.y = static_cast<LONG>(m_screen_center.y);
            ClientToScreen(m_window_handle, &center_point);
            SetCursorPos(center_point.x, center_point.y);
            
            m_mouse_captured = true;
        }
        else if (!capture && m_mouse_captured)
        {
            // Release mouse capture
            ReleaseCapture();
            
            // Show mouse cursor
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
		// Handle mouse wheel event
		// The high 16 bits of wParam contain the wheel delta value
		short wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
		// WHEEL_DELTA is usually 120, normalize it to 1.0 or -1.0
		float normalized_delta = static_cast<float>(wheel_delta) / WHEEL_DELTA;
		m_mouse_wheel_delta += normalized_delta;
    }

    void InputManager::UpdateMousePosition()
    {
        DOLAS_RETURN_IF_FALSE(m_mouse_captured);

        m_previous_mouse_position = m_mouse_position;
        // Get current mouse position
        POINT cursor_pos;
        GetCursorPos(&cursor_pos);
        ScreenToClient(m_window_handle, &cursor_pos);
            
        // Calculate offset relative to screen center
        m_mouse_delta.x = cursor_pos.x - m_screen_center.x;
        m_mouse_delta.y = cursor_pos.y - m_screen_center.y;
            
        // Reset mouse to screen center
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
        // Let ImGui handle the message
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        {
            return true;
        }
    
        // Handle input-related messages
        switch (msg)
        {
            // Keyboard
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
                if (IsKeyUp(static_cast<int>(wParam)))
                {
                    ProduceKeyPressEvent(static_cast<int>(wParam));
                }
                UpdateKeyState(static_cast<int>(wParam), true);
				break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
                if (IsKeyDown(static_cast<int>(wParam)))
                {
                    ProduceKeyReleaseEvent(static_cast<int>(wParam));
                }
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
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

			default:
                break;
        }
        
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace Dolas

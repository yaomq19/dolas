#include "render/dolas_dx_trace.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace Dolas
{
    // Direct3D error helper (ASCII / UTF-8):
    //  - Logs an English message to stderr and to the engine logger (LOG_ERROR)
    //  - Uses FormatMessageA with LANG_ENGLISH to avoid localized messages
    //  - Still breaks into the debugger in Debug builds when HR(x) fails
    HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* file_path,
                            _In_ DWORD         line_number,
                            _In_ HRESULT       hr,
                            _In_opt_ const WCHAR* call_expr)
    {
        char file_utf8[512]        = {};
        char call_utf8[512]        = {};
        char error_message[512]    = {};
        char call_message[512]     = {};
        char full_message[2048]    = {};

        // Convert file path to UTF-8 (or empty string)
        if (file_path && file_path[0] != L'\0')
        {
            WideCharToMultiByte(CP_UTF8, 0, file_path, -1, file_utf8, sizeof(file_utf8), nullptr, nullptr);
        }
        else
        {
            file_utf8[0] = '\0';
        }

        // Convert call expression to UTF-8
        if (call_expr && call_expr[0] != L'\0')
        {
            WideCharToMultiByte(CP_UTF8, 0, call_expr, -1, call_utf8, sizeof(call_utf8), nullptr, nullptr);
            std::snprintf(call_message, sizeof(call_message), "Call expression: %s\n", call_utf8);
        }
        else
        {
            call_message[0] = '\0';
        }

        // Try to get a human readable error message in EN-US
        DWORD res = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            hr,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            error_message,
            static_cast<DWORD>(sizeof(error_message) / sizeof(error_message[0])),
            nullptr);

        if (res == 0)
        {
            // Fallback description if FormatMessageA fails
            std::snprintf(error_message,
                          sizeof(error_message),
                          "Unknown error. HRESULT = 0x%0.8X",
                          static_cast<unsigned int>(hr));
        }
        else
        {
            // Strip trailing CR/LF from FormatMessageA
            char* cr = std::strrchr(error_message, '\r');
            if (cr) *cr = '\0';
            char* lf = std::strrchr(error_message, '\n');
            if (lf) *lf = '\0';
        }

        // Compose final message (ASCII / UTF-8)
        std::snprintf(full_message,
                      sizeof(full_message),
                      "[D3D Error]\nFile: %s\nLine: %lu\nHRESULT: 0x%0.8X\nMessage: %s\n%s",
                      file_utf8,
                      line_number,
                      static_cast<unsigned int>(hr),
                      error_message,
                      call_message);

        DebugBreak();

        return hr;
    }
}
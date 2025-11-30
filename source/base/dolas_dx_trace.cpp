#include "dolas_dx_trace.h"
#include <windows.h>
#include <cstdio>

namespace Dolas
{
    // 更直观的 DX 错误输出：
    // 1. 文本说明输出到标准错误（控制台），而不是 VS Output 窗口；
    // 2. 无论能否通过 FormatMessageW 找到系统错误字符串，都会至少打印十六进制错误码；
    // 3. 可选地弹出对话框并中断到调试器（保持原有行为）。
    HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* file_path,
                            _In_ DWORD         line_number,
                            _In_ HRESULT       hr,
                            _In_opt_ const WCHAR* call_expr,
                            _In_ bool          show_message_box)
    {
        WCHAR line_str[64]       = {};
        WCHAR error_message[512] = {};
        WCHAR full_message[2048] = {};
        WCHAR call_message[512]  = {};

        // 行号字符串
        swprintf_s(line_str, L"%lu", line_number);

        // 尝试从系统获取错误码说明
        DWORD res = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            error_message,
            static_cast<DWORD>(sizeof(error_message) / sizeof(error_message[0])),
            nullptr);

        if (res == 0)
        {
            // 无法识别的错误码，给一个兜底描述
            swprintf_s(error_message,
                       sizeof(error_message) / sizeof(error_message[0]),
                       L"Unknown error, HRESULT = 0x%0.8X",
                       static_cast<unsigned int>(hr));
        }
        else
        {
            // 去掉 FormatMessageW 附带的 \r\n
            WCHAR* cr = wcsrchr(error_message, L'\r');
            if (cr) *cr = L'\0';
            WCHAR* lf = wcsrchr(error_message, L'\n');
            if (lf) *lf = L'\0';
        }

        // 调用表达式信息
        if (call_expr && call_expr[0] != L'\0')
        {
            swprintf_s(call_message,
                       sizeof(call_message) / sizeof(call_message[0]),
                       L"调用表达式：%ls\n",
                       call_expr);
        }

        // 组合一条完整的错误消息
        swprintf_s(full_message,
                   sizeof(full_message) / sizeof(full_message[0]),
                   L"[D3D Error]\n文件：%ls\n行号：%ls\nHRESULT：0x%0.8X\n含义：%ls\n%ls",
                   file_path ? file_path : L"",
                   line_str,
                   static_cast<unsigned int>(hr),
                   error_message,
                   call_message);

        // 输出到标准错误（控制台），方便在非 VS 环境下查看
        fwprintf(stderr, L"%ls\n", full_message);
        fflush(stderr);

        if (show_message_box)
        {
            int nResult = MessageBoxW(GetForegroundWindow(),
                                      full_message,
                                      L"D3D 错误",
                                      MB_YESNO | MB_ICONERROR);
            if (nResult == IDYES)
            {
                DebugBreak();
            }
        }

        return hr;
    }
}
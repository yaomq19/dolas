#ifndef DOLAS_DX_TRACE_H
#define DOLAS_DX_TRACE_H

#include <windows.h>
namespace Dolas
{
    #if defined(DEBUG) | defined(_DEBUG)
    #ifndef HR
    #define HR(x)                                                \
        {                                                            \
            HRESULT hr = (x);                                        \
            if(FAILED(hr))                                            \
            {                                                        \
                DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
            }                                                        \
        }
    #endif
    #else
    #ifndef HR
    #define HR(x) (x)
    #endif 
    #endif

    // ------------------------------
    // DXTraceW函数
    // ------------------------------
    // 在调试输出窗口中输出格式化错误信息，可选的错误窗口弹出(已汉化)
    // [In]strFile            当前文件名，通常传递宏__FILEW__
    // [In]hlslFileName     当前行号，通常传递宏__LINE__
    // [In]hr                函数执行出现问题时返回的HRESULT值
    // [In]strMsg            用于帮助调试定位的字符串，通常传递L#x(可能为NULL)
    // [In]bPopMsgBox       如果为TRUE，则弹出一个消息弹窗告知错误信息
    // 返回值: 形参hr
    HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);
}

#endif // DOLAS_DX_TRACE_H
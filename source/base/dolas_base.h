#ifndef DOLAS_BASE_H
#define DOLAS_BASE_H

// 基础头文件内容将在这里
#define DOLAS_NEW(type, ...) new type(__VA_ARGS__)
#define DOLAS_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

#define DOLAS_RETURN_IF_NULL(ptr) if (!ptr) { return; }
#define DOLAS_RETURN_IF_FALSE(condition) if (!(condition)) { return; }
#define DOLAS_RETURN_IF_TRUE(condition) if ((condition)) { return; }

#define DOLAS_RETURN_TRUE_IF_NULL(ptr) if (!ptr) { return true; }
#define DOLAS_RETURN_TRUE_IF_FALSE(condition) if (!(condition)) { return true; }
#define DOLAS_RETURN_TRUE_IF_TRUE(condition) if ((condition)) { return true; }

#define DOLAS_RETURN_FALSE_IF_NULL(ptr) if (!ptr) { return false; }
#define DOLAS_RETURN_FALSE_IF_FALSE(condition) if (!(condition)) { return false; }
#define DOLAS_RETURN_FALSE_IF_TRUE(condition) if ((condition)) { return false; }

#define DOLAS_RETURN_NULL_IF_NULL(ptr) if (!ptr) { return nullptr; }
#define DOLAS_RETURN_NULL_IF_FALSE(condition) if (!(condition)) { return nullptr; }
#define DOLAS_RETURN_NULL_IF_TRUE(condition) if ((condition)) { return nullptr; }

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

#endif // DOLAS_BASE_H


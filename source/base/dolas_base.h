#ifndef DOLAS_BASE_H
#define DOLAS_BASE_H

namespace Dolas
{
    // Base header file content will be here
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

    #define DOLAS_CONTINUE_IF_NULL(ptr) if (!ptr) { continue; }
    #define DOLAS_CONTINUE_IF_FALSE(condition) if (!(condition)) { continue; }
    #define DOLAS_CONTINUE_IF_TRUE(condition) if ((condition)) { continue; }

    #define DolasInt int
    #define Float float
    #define Double double
    #define Bool bool
    #define UInt unsigned int
    #define ULong uint64_t
    #define Int8 int8_t
    #define Int16 int16_t
    #define Int32 int32_t
    #define Int64 int64_t
    #define Float32 float

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
}
    
#endif // DOLAS_BASE_H


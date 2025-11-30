#ifndef DOLAS_HLSL_SUPPORT_HLSLI
#define DOLAS_HLSL_SUPPORT_HLSLI

// 注意：这里的 b13 要和 C++ 侧绑定 GlobalConstants 使用的槽位保持一致
// 当前实现中，GlobalConstants 被绑定到 D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1（即 13）
#define DOLAS_GLOBAL_CONSTANTS cbuffer GlobalConstants : register(b13)
#endif
#ifndef DOLAS_GLOBAL_CONSTANTS_HLSLI
#define DOLAS_GLOBAL_CONSTANTS_HLSLI

cbuffer PerViewConstantBuffer : register(b0)
{
    matrix g_ViewMatrix;  
    matrix g_ProjectionMatrix;  
}

#endif
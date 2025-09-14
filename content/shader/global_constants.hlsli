#ifndef DOLAS_GLOBAL_CONSTANTS_HLSLI
#define DOLAS_GLOBAL_CONSTANTS_HLSLI

cbuffer PerViewConstantBuffer : register(b0)
{
    matrix g_ViewMatrix;  
    matrix g_ProjectionMatrix;  
    float4 g_CameraPosition;
}

cbuffer PerFrameConstantBuffer : register(b1)
{
    float4 g_LightDirectionIntensity;
    float4 g_LightColor;
}
#endif
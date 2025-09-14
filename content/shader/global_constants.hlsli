#ifndef DOLAS_GLOBAL_CONSTANTS_HLSLI
#define DOLAS_GLOBAL_CONSTANTS_HLSLI

cbuffer PerViewConstantBuffer : register(b0)
{
    matrix g_ViewMatrix;  
    matrix g_ProjectionMatrix;  
    Vector3 g_CameraPosition;
}

cbuffer PerFrameConstantBuffer : register(b1)
{
    Vector4 g_LightDirectionIntensity;
    Vector4 g_LightColor;
}
#endif
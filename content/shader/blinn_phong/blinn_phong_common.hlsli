#ifndef BLINN_PHONG_COMMON_HLSLI
#define BLINN_PHONG_COMMON_HLSLI

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL;
};
#endif
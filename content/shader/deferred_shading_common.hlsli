#ifndef DOLAS_DEFERRED_COMMON_HLSLI
#define DOLAS_DEFERRED_COMMON_HLSLI

struct VS_INPUT
{
    float3 posL : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

#endif
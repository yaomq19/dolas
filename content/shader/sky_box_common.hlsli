#ifndef DOLAS_SKY_BOX_COMMON_HLSLI
#define DOLAS_SKY_BOX_COMMON_HLSLI
struct VS_INPUT
{
    float3 posL : POSITION;
};

struct VS_OUTPUT
{
    float4 posH : SV_POSITION;
    float3 posL : TEXCOORD0;
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float3 posL : TEXCOORD0;
};
#endif
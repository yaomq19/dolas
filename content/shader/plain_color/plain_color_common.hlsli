#ifndef DOLAS_PLAIN_COLOR_COMMON_HLSLI
#define DOLAS_PLAIN_COLOR_COMMON_HLSLI

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 gbuffer_a : SV_TARGET0;
    float4 gbuffer_b : SV_TARGET1;
    float4 gbuffer_c : SV_TARGET2;
};

#endif
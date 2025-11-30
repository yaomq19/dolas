#ifndef DEBUG_DRAW_COMMON_HLSLI
#define DEBUG_DRAW_COMMON_HLSLI

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
    float4 output_color : SV_TARGET0;
};

#endif
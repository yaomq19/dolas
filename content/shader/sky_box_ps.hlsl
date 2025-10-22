// 参考 deferred_shading_ps.hlsl
#include "global_constants.hlsli"

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
Texture2D sky_box_map : register(t0);
SamplerState sky_box_map_sampler : register(s0);

Texture2D depth_map : register(t1);
SamplerState depth_map_sampler : register(s1);

// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET0
{   
    float4 sky_box_color = sky_box_map.Sample(sky_box_map_sampler, input.texcoord);

    return sky_box_color;
}
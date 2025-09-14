#include "solid_common.hlsli"
#include "global_constants.hlsli"

Texture2D base_color_map : register(t0);
SamplerState base_color_map_sampler : register(s0);

// Simple color output
PS_OUTPUT PS(PS_INPUT input)
{   
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.gbuffer_a = float4(1.0, 1.0, 1.0, 1.0);
    float4 base_color = base_color_map.Sample(base_color_map_sampler, input.texcoord);
    output.gbuffer_b = base_color;
    output.gbuffer_c = float4(0.8f, 0.5f, 1.0f, 0.5f);
    return output;
}
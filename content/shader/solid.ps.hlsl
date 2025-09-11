#include "solid_common.hlsli"
#include "global_constants.hlsli"
// Simple color output
PS_OUTPUT PS(PS_INPUT input)
{   
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.gbuffer_a = float4(1.0f, 0.0f, 0.0f, 0.5f);
    output.gbuffer_b = float4(0.0f, 1.0f, 0.0f, 0.5f);
    output.gbuffer_c = float4(0.0f, 0.0f, 1.0f, 0.5f);
    return output;
}
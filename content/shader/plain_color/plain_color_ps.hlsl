#include "plain_color/plain_color_common.hlsli"
#include "global_constants.hlsli"
#include "dolas_hlsl_support.hlsli"
DOLAS_GLOBAL_CONSTANTS
{
    float4 g_PlainColor;
}
// Simple color output
PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.gbuffer_a = float4(1.0, 1.0, 1.0, 1.0);
    output.gbuffer_b = g_PlainColor;
    output.gbuffer_c = float4(0.8f, 0.5f, 1.0f, 0.5f);
    return output;
}
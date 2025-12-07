#include "debug_draw/debug_draw_common.hlsli"
#include "global_constants.hlsli"
#include "dolas_hlsl_support.hlsli"
DOLAS_GLOBAL_CONSTANTS
{
    float4 g_DebugDrawColor;
}
// Simple color output
PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.output_color = g_DebugDrawColor;
    return output;
}
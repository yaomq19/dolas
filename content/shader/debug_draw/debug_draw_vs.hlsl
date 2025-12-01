#include "debug_draw_common.hlsli"
#include "../global_constants.hlsli"

// Basic vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 world_position = mul(float4(input.position, 1.0f), g_WorldMatrix);
    float4 view_position = mul(world_position, g_ViewMatrix);
    float4 proj_position = mul(view_position, g_ProjectionMatrix);
    
    output.position = proj_position;
    return output;
}

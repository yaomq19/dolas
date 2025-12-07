#include "solid/solid_common.hlsli"
#include "global_constants.hlsli"

// Basic vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 world_position = float4(input.position, 1.0f);
    float4 view_position = mul(world_position, g_ViewMatrix);
    float4 proj_position = mul(view_position, g_ProjectionMatrix);
    
    output.position = proj_position;
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    return output;
}

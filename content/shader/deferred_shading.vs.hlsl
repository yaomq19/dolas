#include "deferred_shading_common.hlsli"

// Basic vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.posH = float4(input.posL, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}

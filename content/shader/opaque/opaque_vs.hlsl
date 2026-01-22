#include "opaque/opaque_common.hlsli"
#include "global_constants.hlsli"

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    // 1. 变换顶点位置到裁切空间
    float4 world_pos = mul(float4(input.position, 1.0f), g_WorldMatrix);
    float4 view_pos  = mul(world_pos, g_ViewMatrix);
    output.position  = mul(view_pos, g_ProjectionMatrix);

    // 2. 传递纹理坐标
    output.texcoord = input.texcoord;

    // 3. 将法线和切线变换到世界空间 (使用世界矩阵的旋转部分)
    // 注意：非等比缩放需要使用逆转置矩阵，这里假设是等比缩放
    output.world_normal  = normalize(mul(input.normal, (float3x3)g_WorldMatrix));
    output.world_tangent = normalize(mul(input.tangent, (float3x3)g_WorldMatrix));

    // 4. 计算副切线 (Bitangent)
    output.world_bitangent = normalize(cross(output.world_normal, output.world_tangent));

    return output;
}
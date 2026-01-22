#include "opaque/opaque_common.hlsli"
#include "global_constants.hlsli"
#include "shade_mode.hlsli"
#include "dolas_hlsl_support.hlsli"

// 定义材质参数（根据你的 MaterialManager，slot 0 是 albedo，slot 1 是 normal）
Texture2D g_albedo_map : register(t0);
Texture2D g_normal_map : register(t1);
SamplerState g_sampler  : register(s0);

// 定义材质属性常量
DOLAS_GLOBAL_CONSTANTS
{
    float4 k_a;             // 材质环境光颜色
    float4 k_d;             // 材质漫反射颜色
    float4 k_s_shininess;   // rgb: 镜面反射, a: 高光指数
}

float3 EncodeNormal(float3 n)
{
    return n * 0.5f + 0.5f;
}

PS_OUTPUT PS(PS_INPUT input)
{
    float3 N = normalize(input.world_normal);
    float3 T = normalize(input.world_tangent);
    float3 B = normalize(input.world_bitangent);
    float3x3 TBN = float3x3(T, B, N);

    float3 tangent_normal = g_normal_map.Sample(g_sampler, input.texcoord).rgb * 2.0f - 1.0f;
    float3 world_normal = normalize(mul(tangent_normal, TBN));

    PS_OUTPUT output = (PS_OUTPUT)0;
    
    output.gbuffer_a = float4(EncodeNormal(world_normal), 1.0f);
    
    float4 albedo = g_albedo_map.Sample(g_sampler, input.texcoord);
    output.gbuffer_b = float4(albedo.rgb, 1.0f);
    
    float shade_mode_float = EncodeShadeMode(SHADE_MODE_OPAQUE);
    output.gbuffer_c = float4(1.0f, 1.0f, 1.0f, shade_mode_float);
    
    output.gbuffer_d = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return output;
}
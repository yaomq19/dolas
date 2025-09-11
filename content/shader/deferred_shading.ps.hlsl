#include "deferred_shading_common.hlsli"
#include "global_constants.hlsli"

Texture2D g_gbuffer_a : register(t0);
SamplerState g_sampler_a : register(s0);

Texture2D g_gbuffer_b : register(t1);
SamplerState g_sampler_b : register(s1);

Texture2D g_gbuffer_c : register(t2);
SamplerState g_sampler_c : register(s2);

// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET0
{   
    float4 color_a = g_gbuffer_a.Sample(g_sampler_a, input.texcoord);
    float4 color_b = g_gbuffer_b.Sample(g_sampler_b, input.texcoord);
    float4 color_c = g_gbuffer_c.Sample(g_sampler_c, input.texcoord);

    float3 world_normal = color_a.xyz * 2.0f - 1.0f;
    float3 base_color = color_b.xyz;

    float metallic = color_c.x;
    float dialetic_specular_multiplier = color_c.y;
    float roughness = color_c.z;

    float3 albedo = base_color * (1.0f - metallic);
    float3 specular = lerp(float3(0.04f * dialetic_specular_multiplier, 0.04f * dialetic_specular_multiplier, 0.04f * dialetic_specular_multiplier), base_color, metallic);
    float3 ambient = float3(0.01f, 0.01f, 0.01f);

    float3 final_color = albedo + specular + ambient;

    return float4(final_color, 1.0f);
}
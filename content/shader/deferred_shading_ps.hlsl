#include "deferred_shading_common.hlsli"
#include "global_constants.hlsli"
#include "surface_common.hlsli"
#include "light.hlsli"

Texture2D g_gbuffer_a : register(t0);
SamplerState g_sampler_a : register(s0);

Texture2D g_gbuffer_b : register(t1);
SamplerState g_sampler_b : register(s1);

Texture2D g_gbuffer_c : register(t2);
SamplerState g_sampler_c : register(s2);

Texture2D depth_map : register(t3);
SamplerState depth_map_sampler : register(s3);

float3 reconstruct_world_position(float2 texcoord, float depth)
{
    float4 ndc_position = float4(texcoord * 2.0f - 1.0f, depth, 1.0f);
    float4 world_position = mul(ndc_position, g_ViewMatrix);
    return world_position.xyz;
}

// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET0
{   
    float4 color_a = g_gbuffer_a.Sample(g_sampler_a, input.texcoord);
    float4 color_b = g_gbuffer_b.Sample(g_sampler_b, input.texcoord);
    float4 color_c = g_gbuffer_c.Sample(g_sampler_c, input.texcoord);
    float depth = depth_map.Sample(depth_map_sampler, input.texcoord).x;

    float3 world_position = reconstruct_world_position(input.texcoord, depth);

    float3 world_normal = color_a.xyz * 2.0f - 1.0f;
    float3 base_color = color_b.xyz;

    float metallic = color_c.x;
    float dialetic_specular_multiplier = color_c.y;
    float roughness = color_c.z;
    int shade_mode = SHADE_MODE_STANDARD;

    float3 albedo = base_color * (1.0f - metallic);
    float3 specular = lerp(float3(0.04f * dialetic_specular_multiplier, 0.04f * dialetic_specular_multiplier, 0.04f * dialetic_specular_multiplier), base_color, metallic);
    
    SurfaceData surface_data = (SurfaceData)0;
    surface_data.albedo = albedo;
    surface_data.specular = specular;
    surface_data.metallic = metallic;
    surface_data.roughness = roughness;
    surface_data.world_position = world_position;
    surface_data.world_normal = world_normal;
    surface_data.shade_mode = shade_mode;
    
    LightData light_data = (LightData)0;
    light_data.direction = g_LightDirectionIntensity.xyz;
    light_data.intensity = g_LightDirectionIntensity.w;
    light_data.color = g_LightColor.xyz;

    float3 ambient_shading = float3(0.01f, 0.01f, 0.01f);
    float3 main_light_shading = MainLightShading(surface_data, light_data);

    float3 final_color = main_light_shading + ambient_shading;

    return float4(final_color, 1.0f);
}
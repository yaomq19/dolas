#include "deferred_shading/deferred_shading_common.hlsli"
#include "global_constants.hlsli"
#include "surface_common.hlsli"
#include "light.hlsli"

Texture2D g_gbuffer_a : register(t0);
SamplerState g_sampler_a : register(s0);

Texture2D g_gbuffer_b : register(t1);
SamplerState g_sampler_b : register(s1);

Texture2D g_gbuffer_c : register(t2);
SamplerState g_sampler_c : register(s2);

Texture2D g_gbuffer_d : register(t3);
SamplerState g_sampler_d : register(s3);

Texture2D g_depth_map : register(t4);
SamplerState g_depth_map_sampler : register(s4);

struct GBufferData
{
    float4 gbuffer_a;
    float4 gbuffer_b;
    float4 gbuffer_c;
    float4 gbuffer_d;
};

void DecodeGBufferData(inout GBufferData gbuffer_data, float2 texcoord)
{
    gbuffer_data.gbuffer_a = g_gbuffer_a.Sample(g_sampler_a, texcoord);
    gbuffer_data.gbuffer_b = g_gbuffer_b.Sample(g_sampler_b, texcoord);
    gbuffer_data.gbuffer_c = g_gbuffer_c.Sample(g_sampler_c, texcoord);
    gbuffer_data.gbuffer_d = g_gbuffer_d.Sample(g_sampler_d, texcoord);
}

float3 DecodeNormal(float3 normal_positive)
{
    return normalize(normal_positive * 2.0f - 1.0f);
}

float DecodeShininess(float shininess_positive)
{
    return shininess_positive * 255.0f;
}

void ConvertGBufferToSurfaceData(inout SurfaceData surface_data, in GBufferData gbuffer_data)
{
    float shade_mode_float = gbuffer_data.gbuffer_c.w;
    int shade_mode = DecodeShadeMode(shade_mode_float);
    surface_data.shade_mode = shade_mode;

    if (shade_mode == SHADE_MODE_STANDARD)
    {
        
    }
    else if (shade_mode == SHADE_MODE_BLINN_PHONG)
    {
        surface_data.world_normal = DecodeNormal(gbuffer_data.gbuffer_a.xyz);
        surface_data.k_a = gbuffer_data.gbuffer_b.rgb;
        surface_data.k_d = gbuffer_data.gbuffer_c.rgb;
        surface_data.k_s = gbuffer_data.gbuffer_d.rgb;
        surface_data.shininess = DecodeShininess(gbuffer_data.gbuffer_d.a);
    }
}

float3 reconstruct_world_position(float2 texcoord, float depth)
{
    float4 ndc_position = float4(texcoord * 2.0f - 1.0f, depth, 1.0f);
    float4 world_position = mul(ndc_position, g_ViewMatrix);
    return world_position.xyz;
}

float4 PS(PS_INPUT input) : SV_TARGET0
{   
    GBufferData gbuffer_data = (GBufferData)0;
    DecodeGBufferData(gbuffer_data, input.texcoord);

    SurfaceData surface_data = (SurfaceData)0;
    ConvertGBufferToSurfaceData(surface_data, gbuffer_data);

    float depth = g_depth_map.Sample(g_depth_map_sampler, input.texcoord).x;
    float3 world_position = reconstruct_world_position(input.texcoord, depth);
    surface_data.world_position = world_position;
    
    LightData light_data = (LightData)0;
    light_data.direction = g_LightDirectionIntensity.xyz;
    light_data.intensity = g_LightDirectionIntensity.w;
    light_data.color = g_LightColor.xyz;

    float3 N = normalize(surface_data.world_normal);
    // float3 L = light_data.direction;
    float3 L = float3(0.0f, 1.0f, 0.0f);
    float3 V = normalize(g_EyeDirection.xyz);

    SurfaceContext surface_context = EvaluateSurfaceContext(N, L, V);
    
    float3 main_light_shading = MainLightShading(surface_data, surface_context, light_data);
    float3 I_a = float3(0.01f, 0.01f, 0.01f);
    float3 ambient_lighting = surface_data.k_a * I_a;
    float3 final_color = main_light_shading + ambient_lighting;
    return float4(final_color, 1.0f);
}
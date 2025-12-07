#include "blinn_phong/blinn_phong_common.hlsli"
#include "global_constants.hlsli"
#include "shade_mode.hlsli"
#include "dolas_hlsl_support.hlsli"
DOLAS_GLOBAL_CONSTANTS
{
    float4 k_a;
    float4 k_d;
    float4 k_s_shininess;
}

float3 EncodeNormal(float3 normal)
{
    return normalize(normal) * 0.5f + 0.5f;
}

float EncodeShininess(float shininess)
{
    return shininess / 255.0f;
}

// Basic pixel shader
PS_OUTPUT PS(PS_INPUT input)
{
    int shade_mode = SHADE_MODE_BLINN_PHONG;
    float shade_mode_float = EncodeShadeMode(shade_mode);
    float3 normal_positive = EncodeNormal(input.normal);
    float shininess_positive = EncodeShininess(k_s_shininess.a);

    PS_OUTPUT output = (PS_OUTPUT)0;
    output.gbuffer_a = float4(normal_positive, 1.0f);
    output.gbuffer_b = float4(k_a.rgb, 0.0f);
    output.gbuffer_c = float4(k_d.rgb, shade_mode_float);
    output.gbuffer_d = float4(k_s_shininess.rgb, shininess_positive);
    return output;
}
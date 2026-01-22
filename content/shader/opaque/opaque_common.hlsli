#ifndef DOLAS_OPAQUE_COMMON_HLSLI
#define DOLAS_OPAQUE_COMMON_HLSLI

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT; // 必须有切线才能构建 TBN
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 world_normal  : NORMAL;
    float3 world_tangent : TANGENT;
    float3 world_bitangent : BITANGENT;
};

// 像素着色器输入与顶点着色器输出一致
typedef VS_OUTPUT PS_INPUT;

struct PS_OUTPUT
{
    float4 gbuffer_a : SV_TARGET0; // Normal
    float4 gbuffer_b : SV_TARGET1; // Albedo / Ambient
    float4 gbuffer_c : SV_TARGET2; // Diffuse / ShadeMode
    float4 gbuffer_d : SV_TARGET3; // Specular / Shininess
};

#endif
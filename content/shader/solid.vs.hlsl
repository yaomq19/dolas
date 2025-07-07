#include "solid_common.hlsli"

// 常量缓冲区
cbuffer ConstantBuffer : register(b0)
{
    float4x4 worldViewProj;
};

// Basic vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    //output.position = mul(float4(input.position, 1.0f), worldViewProj);
    output.position = float4(input.position, 1.0f);
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    return output;
}

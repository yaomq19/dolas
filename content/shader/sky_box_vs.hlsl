// 参考 deferred_shading_vs.hlsl
struct VS_INPUT
{
    float3 posL : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

// Basic vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.posH = float4(input.posL, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}
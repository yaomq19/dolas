// Example HLSL Shader with multiple entry points
// This demonstrates the CompileShaders target functionality

// Input structures
struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
};

// Constant buffer
cbuffer PerFrame : register(b0)
{
    matrix worldViewProj;
    float4 lightDir;
};

// ============ Vertex Shader Entry Points ============

// Basic vertex shader
void VS_Entry_Basic(VS_INPUT input, out PS_INPUT output)
{
    output.position = mul(float4(input.position, 1.0f), worldViewProj);
    output.normal = input.normal;
    output.texcoord = input.texcoord;
}

// Advanced vertex shader with lighting calculations
void VS_Entry_Advanced(VS_INPUT input, out PS_INPUT output)
{
    output.position = mul(float4(input.position, 1.0f), worldViewProj);
    output.normal = normalize(input.normal);
    output.texcoord = input.texcoord * 2.0f; // Scale texture coordinates
}

// ============ Pixel Shader Entry Points ============

// Simple color output
void PS_Entry_Solid(PS_INPUT input, out float4 color : SV_TARGET)
{
    color = float4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
}

// Textured output
void PS_Entry_Textured(PS_INPUT input, out float4 color : SV_TARGET)
{
    // This would normally sample a texture
    color = float4(input.texcoord, 0.0f, 1.0f);
}

// Lighting calculation
void PS_Entry_Lit(PS_INPUT input, out float4 color : SV_TARGET)
{
    float3 normal = normalize(input.normal);
    float NdotL = max(0.0f, dot(normal, -lightDir.xyz));
    color = float4(NdotL, NdotL, NdotL, 1.0f);
}

// ============ Compute Shader Entry Points ============

RWTexture2D<float4> OutputTexture : register(u0);

// Simple compute shader
[numthreads(8, 8, 1)]
void CS_Entry_Generate(uint3 id : SV_DispatchThreadID)
{
    OutputTexture[id.xy] = float4(id.x / 255.0f, id.y / 255.0f, 0.0f, 1.0f);
}

// Advanced compute shader
[numthreads(16, 16, 1)]
void CS_Entry_Filter(uint3 id : SV_DispatchThreadID)
{
    float4 color = OutputTexture[id.xy];
    color.rgb = color.rgb * 0.5f + 0.5f; // Brighten
    OutputTexture[id.xy] = color;
} 
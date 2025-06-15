// 测试着色器文件
// 用于渲染带纹理的正方形

// 顶点着色器输入结构
struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

// 顶点着色器输出结构（也是像素着色器输入）
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

// 纹理和采样器声明
Texture2D albedoTexture : register(t0);
SamplerState textureSampler : register(s0);

// ============ Vertex Shader Entry Points ============

// 顶点着色器
PS_INPUT VS_Entry_Test(VS_INPUT input)
{
    PS_INPUT output;
    
    // 直接传递位置（假设已经在裁剪空间）
    output.position = float4(input.position, 1.0f);
    
    // 传递纹理坐标
    output.texcoord = input.texcoord;
    
    return output;
}

// ============ Pixel Shader Entry Points ============

// 像素着色器
float4 PS_Entry_Test(PS_INPUT input) : SV_TARGET
{
    // 采样纹理
    float4 textureColor = albedoTexture.Sample(textureSampler, input.texcoord);
    
    // 如果没有纹理，返回红色作为调试
    if (any(textureColor.rgb == 0.0f))
    {
        return float4(1.0f, 0.0f, 0.0f, 1.0f); // 红色
    }
    
    return textureColor;
}

// ============ Compute Shader Entry Points ============

// Simple compute shader for testing
[numthreads(8, 8, 1)]
void CS_Entry_Test(uint3 id : SV_DispatchThreadID)
{
    // Simple compute operation for testing
}

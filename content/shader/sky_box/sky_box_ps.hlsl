// 参考 deferred_shading_ps.hlsl
#include "global_constants.hlsli"
#include "sky_box/sky_box_common.hlsli"
Texture2D sky_box_map : register(t0);
SamplerState sky_box_map_sampler : register(s0);

static const float PI = 3.14159265359;
static const float EPSILON = 1e-6;  // 更精确的浮点数阈值

float2 get_spherical_uv_1(float3 dir)
{
    float u = atan2(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = asin(dir.y) / PI + 0.5;
    return float2(u, v);
}

float2 get_spherical_uv_2(float3 dir)
{
    float theta = acos(dir.z);
    float temp2 = sin(theta);
    float temp = dir.x / temp2;
    float phi = acos(temp);
    float u = phi / (2.0 * PI);
    if (dir.y < 0.0f)
    {
        u = 1.0f - u;
    }
    float v = acos(dir.z) / PI;
    return float2(u, v);
}

float2 get_spherical_uv_3(float3 dir)
{
    float theta = 0.0f;
    [flatten]
    if (abs(dir.z - 1.0f) < EPSILON) theta = 0.0f;
    else if (abs(dir.z + 1.0f) < EPSILON) theta = PI;
    else theta = acos(dir.z);

    float phi = 0.0f;
    float temp = dir.x / sin(theta);
    [flatten]
    if (abs(temp - 1.0f) < EPSILON) phi = 0.0f;
    else if (abs(temp + 1.0f) < EPSILON) phi = PI;
    else phi = acos(temp);

    float u = phi / (2.0 * PI);
    if (dir.y < 0.0f)
    {
        u = 1.0f - u;
    }
    float v = acos(dir.z) / PI;
    return float2(u, v);
}

float2 get_spherical_uv_4(float3 dir)
{
    // 计算极角 theta（与z轴的夹角）
    float theta = 0.0f;
    [flatten]
    if (abs(dir.z - 1.0f) < EPSILON) 
        theta = 0.0f;
    else if (abs(dir.z + 1.0f) < EPSILON) 
        theta = PI;
    else 
        theta = acos(clamp(dir.z, -1.0f, 1.0f));
    
    // 计算方位角 phi
    // 使用 atan2 更可靠，它会自动处理所有象限
    float phi = atan2(dir.y, dir.x);  // 范围 [-PI, PI]
    
    // 转换到 [0, 1] 范围
    float u = phi / (2.0 * PI) + 0.5;  // 将 [-PI, PI] 映射到 [0, 1]
    float v = theta / PI;               // 将 [0, PI] 映射到 [0, 1]
    
    return float2(u, v);
}
// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET0
{   
    // 归一化方向向量（确保是单位向量）
    float3 dir = normalize(input.posL);
    
    float2 uv = get_spherical_uv_4(dir);
    // 采样全景图
    float4 sky_box_color = sky_box_map.Sample(sky_box_map_sampler, uv);

    return sky_box_color;
}
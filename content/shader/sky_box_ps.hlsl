// 参考 deferred_shading_ps.hlsl
#include "global_constants.hlsli"
#include "sky_box_common.hlsli"
Texture2D sky_box_map : register(t0);
SamplerState sky_box_map_sampler : register(s0);

Texture2D depth_map : register(t1);
SamplerState depth_map_sampler : register(s1);

// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET0
{   
    // 归一化方向向量（确保是单位向量）
    float3 dir = normalize(input.posL);
    
    // 从笛卡尔坐标转换到球面坐标（经纬映射）
    // 水平方位角: atan2(z, x) 范围 [-π, π]
    // 垂直极角: asin(y) 范围 [-π/2, π/2]
    const float PI = 3.14159265359;
    
    // 计算 UV 坐标
    // u: 水平方向，从左到右对应 [0, 1]
    // v: 垂直方向，从上到下对应 [0, 1]
    float u = atan2(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = asin(dir.y) / PI + 0.5;
    
    float2 uv = float2(u, v);
    
    // 采样全景图
    float4 sky_box_color = sky_box_map.Sample(sky_box_map_sampler, uv);

    return sky_box_color;
}
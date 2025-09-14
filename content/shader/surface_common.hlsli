#ifndef SURFACE_COMMON_HLSLI
#define SURFACE_COMMON_HLSLI

struct SurfaceData
{
    float3 albedo;
    float3 specular;
    float roughness;
    float metallic;
    float3 world_position;
    float3 world_normal;
    int shade_mode;
};

#endif
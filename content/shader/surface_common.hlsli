#ifndef SURFACE_COMMON_HLSLI
#define SURFACE_COMMON_HLSLI

struct SurfaceData
{
    // common
    float3 world_normal;
    
    // pbr
    float3 albedo;
    float3 specular;
    float roughness;
    float metallic;
    float3 world_position;

    // blinn-phong
    float3 k_a;
    float3 k_d;
    float3 k_s;

    // shade mode
    int shade_mode;
};

#endif
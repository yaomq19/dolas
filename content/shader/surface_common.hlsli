#ifndef SURFACE_COMMON_HLSLI
#define SURFACE_COMMON_HLSLI

struct SurfaceContext
{
    float NoL;
    float NoH;
    float NoV;
    float LoH;
    float VoH;
    float VoL;
    float VoN;
};

SurfaceContext EvaluateSurfaceContext(float3 N, float3 L, float3 V)
{
    SurfaceContext surface_context = (SurfaceContext)0;
    float3 H = normalize(L + V);
    surface_context.NoL = dot(N, L);
    surface_context.NoH = dot(N, H);
    surface_context.NoV = dot(N, V);
    surface_context.LoH = dot(L, H);
    surface_context.VoH = dot(V, H);
    surface_context.VoL = dot(V, L);
    surface_context.VoN = dot(V, N);
    return surface_context;
}
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
    float shininess;

    // shade mode
    int shade_mode;
};

#endif
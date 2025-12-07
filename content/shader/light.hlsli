#ifndef LIGHT_HLSLI
#define LIGHT_HLSLI
#include "global_constants.hlsli"
#include "surface_common.hlsli"
#include "shade_mode.hlsli"

struct LightData
{
    float3 direction;
    float intensity;
    float3 color;
};

struct LightAccumulation
{
    float3 diffuse;
    float3 specular;
};

float3 EvaluateDiffuse(SurfaceData surface_data)
{
    return surface_data.albedo;
}

float3 EvaluateSpecular(SurfaceData surface_data)
{
    return surface_data.specular;
}

LightAccumulation EvaluateStandardBxDF(SurfaceData surface_data)
{
    LightAccumulation light_accumulation = (LightAccumulation)0;
    light_accumulation.diffuse = EvaluateDiffuse(surface_data);
    light_accumulation.specular = EvaluateSpecular(surface_data);
    return light_accumulation;
}

LightAccumulation EvaluateBlinnPhongBxDF(SurfaceData surface_data)
{
    LightAccumulation light_accumulation = (LightAccumulation)0;
    light_accumulation.diffuse = EvaluateDiffuse(surface_data);
    light_accumulation.specular = EvaluateSpecular(surface_data);
    return light_accumulation;
}

LightAccumulation EvaluateBxDF(SurfaceData surface_data)
{
    LightAccumulation light_accumulation = (LightAccumulation)0;

    if (surface_data.shade_mode == SHADE_MODE_STANDARD)
    {
        light_accumulation = EvaluateStandardBxDF(surface_data);
    }
    else if (surface_data.shade_mode == SHADE_MODE_BLINN_PHONG)
    {
        light_accumulation = EvaluateBlinnPhongBxDF(surface_data);
    }
    else
    {
        light_accumulation = (LightAccumulation)0;
    }
    return light_accumulation;
}

float3 MainLightShading(SurfaceData surface_data, LightData light_data)
{
    LightAccumulation light_accumulation = EvaluateBxDF(surface_data);
    return light_accumulation.diffuse * light_data.color * light_data.intensity + light_accumulation.specular * light_data.color * light_data.intensity;
}
#endif
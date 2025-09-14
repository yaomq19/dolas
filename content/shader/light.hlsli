#ifndef LIGHT_HLSLI
#define LIGHT_HLSLI
#include "global_constants.hlsli"
#include "surface_common.hlsli"

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

LightAccumulation EvaluateBxDF(SurfaceData surface_data, LightData light_data)
{
    LightAccumulation light_accumulation = (LightAccumulation)0;

    switch (surface_data.shade_mode)
    {
        case SurfaceType::Diffuse:
            light_accumulation.diffuse = EvaluateDiffuse(surface_data, light_data);
            break;
        case SurfaceType::Specular:
            light_accumulation.specular = EvaluateSpecular(surface_data, light_data);
            break;
    }
    return light_accumulation;
}

float3 MainLightShading(SurfaceData surface_data, LightData light_data)
{
    

    LightAccumulation light_accumulation = EvaluateBxDF(surface_data, light_data);

    return light_accumulation.diffuse + light_accumulation.specular;

}
#endif
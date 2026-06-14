#include "deferred_shading/deferred_shading_common.hlsli"

Texture2D g_scene_result : register(t0);
SamplerState g_scene_result_sampler : register(s0);

float4 PS(PS_INPUT input) : SV_TARGET0
{
    return g_scene_result.Sample(g_scene_result_sampler, input.texcoord);
}

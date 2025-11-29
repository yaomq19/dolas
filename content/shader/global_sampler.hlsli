#ifndef DOLAS_GLOBAL_SAMPLER_HLSLI
#define DOLAS_GLOBAL_SAMPLER_HLSLI

SamplerState g_linear_sampler : register(s15);
SamplerState g_clamp_sampler : register(s14);
SamplerState g_wrap_sampler : register(s13);

#endif
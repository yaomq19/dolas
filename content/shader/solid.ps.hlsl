#include "solid_common.hlsli"

// Simple color output
float4 PS(PS_INPUT input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 0.5f);
}
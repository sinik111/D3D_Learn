#include "Shared.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
    return saturate(texDiffuse.Sample(samLinear, input.tex) * dot((float3) -lightDir, input.norm) * lightColor);
}
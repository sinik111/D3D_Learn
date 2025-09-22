#include "Shared.hlsli"

float4 main(PS_SKYBOX_INPUT input) : SV_Target
{
    return texCubeDiffuse.Sample(samLinear, input.originPos);
}
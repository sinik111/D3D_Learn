#include "Shared.hlsli"

TextureCube texCubeDiffuse : register(t0);

float4 main(PS_SKYBOX_INPUT input) : SV_Target
{
    return texCubeDiffuse.Sample(samLinear, input.originPos);
}
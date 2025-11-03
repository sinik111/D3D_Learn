#include "Shared.hlsli"

TextureCube g_texCubeDiffuse : register(t1);

float4 main(PS_INPUT_SKYBOX input) : SV_Target
{
    return g_texCubeDiffuse.Sample(g_samLinear, input.localPos);
}
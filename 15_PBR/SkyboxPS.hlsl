#include "Shared.hlsli"

TextureCube g_texCubeDiffuse : register(t0);

float4 main(PS_INPUT_SKYBOX input) : SV_Target
{
    float3 color = g_texCubeDiffuse.Sample(g_samLinear, input.localPos).rgb;
    color = pow(abs(color), 1.0f / 2.2f);
    return float4(color, 1.0f);
}
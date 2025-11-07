#include "Shared.hlsli"

Texture2D g_texOpacity : register(t0);

void main(PS_INPUT input)
{
    float4 texOpacColor = g_texOpacity.Sample(g_samLinear, input.tex);
    
    clip(texOpacColor.a - 0.5f);
    
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
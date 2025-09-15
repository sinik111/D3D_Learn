#include "Shared.hlsli"

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = saturate(dot((float3) lightDir, input.norm) * lightColor);
    
    finalColor.a = 1.0f;
    
    return finalColor;
}
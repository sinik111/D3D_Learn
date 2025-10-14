#include "Shared.hlsli"

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texSpecular : register(t2);
Texture2D texEmissive : register(t3);
Texture2D texOpacity : register(t4);

float4 main(PS_INPUT input) : SV_Target
{   
    clip(texOpacity.Sample(samLinear, input.tex).a - 0.5f);
    
    float4 diffuseColor = texDiffuse.Sample(samLinear, input.tex);
    
    float4 ambient = diffuseColor * ambientLightColor;
    float4 diffuse = diffuseColor * max(dot((float3) -lightDir, normalize(input.norm)), 0.0f) * lightColor;
    
    return saturate(ambient + diffuse);
}
#include "Shared.hlsli"

SamplerState g_samPoint : register(s0);

Texture2D g_texRamp : register(t0);

float4 main(PS_INPUT input) : SV_Target
{
    float3 N = normalize(input.normal);
    
    float3 V = normalize(-input.viewPos);
    
    float3 L = normalize(input.viewLightDir);
    
    float3 R = 2.0f * dot(N, L) * N - L;
    
    float4 ambient = g_materialAmbient * g_ambientLightColor;
    
    float diffuseIntensity = saturate(dot(N, L));
    
    float2 rampUV = float2(diffuseIntensity, 0.5f);
    float4 rampColor = g_texRamp.Sample(g_samPoint, rampUV);
    
    float4 diffuse = rampColor * g_materialDiffuse * g_lightColor * diffuseIntensity;
    
    float specularIntensity = diffuseIntensity > 0.0f ? pow(saturate(dot(R, V)), g_shininess) : 0.0f;
    float4 specular = g_materialSpecular * g_lightColor * specularIntensity;
    
    float4 emissive = 0.0f;
    if (input.localPos.y > g_emissivePosition)
    {
        emissive = g_materialEmissive;
    }
   
    return saturate(ambient + diffuse + specular + emissive);
}
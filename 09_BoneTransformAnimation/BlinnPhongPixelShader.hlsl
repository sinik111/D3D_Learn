#include "Shared.hlsli"

Texture2D g_texDiffuse : register(t0);
Texture2D g_texNormal : register(t1);
Texture2D g_texSpecular : register(t2);
Texture2D g_texEmissive : register(t3);
Texture2D g_texOpacity : register(t4);

float4 main(PS_INPUT input) : SV_Target
{
    float4 texDiffColor = g_texDiffuse.Sample(g_samLinear, input.tex);
    float4 texNormColor = g_texNormal.Sample(g_samLinear, input.tex);
    float4 texSpecColor = g_texSpecular.Sample(g_samLinear, input.tex);
    float4 texOpacColor = g_texOpacity.Sample(g_samLinear, input.tex);
    
    clip(texOpacColor.a - 0.5f);
    
    // emissive
    float4 emissive = g_texEmissive.Sample(g_samLinear, input.tex);
    
    // normal
    float3 norm = normalize(input.norm);
    float3 tan = normalize(input.tan);
    float3 binorm = normalize(input.binorm);
    float3x3 tbnMatrix = float3x3(tan, binorm, norm);
    
    float3 texNorm = DecodeNormal(texNormColor.rgb);
    float3 worldNorm = normalize(mul(texNorm, tbnMatrix));
    
    // ambient
    float4 ambient = texDiffColor * g_materialAmbient * g_ambientLightColor;
    
    // diffuse
    float diffuseScalar = max(dot(worldNorm, -g_lightDir.xyz), 0.0f);
    float4 diffuse = texDiffColor * g_lightColor * diffuseScalar;
    
    // specular
    float3 viewDir = normalize(g_cameraPos.xyz - input.worldPos);
    float3 halfVector = normalize(-g_lightDir.xyz + viewDir);
    float specularScalar = max(dot(worldNorm, halfVector), 0.0f) * step(0.000001f, max(dot(norm, -g_lightDir.xyz), 0.0f));
    float4 specular = texSpecColor * g_materialSpecular * g_lightColor * pow(specularScalar, g_shininess.x);
    
    float4 finalColor = saturate(diffuse + ambient + specular + emissive);
    finalColor.a = texOpacColor.a;
    
    return finalColor;
}
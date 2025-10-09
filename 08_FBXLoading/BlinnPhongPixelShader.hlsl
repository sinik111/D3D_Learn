#include "Shared.hlsli"

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texSpecular : register(t2);
Texture2D texEmissive : register(t3);
Texture2D texOpacity : register(t4);

float4 main(PS_INPUT input) : SV_Target
{
    float4 texDiffColor = texDiffuse.Sample(samLinear, input.tex);
    float4 texNormColor = texNormal.Sample(samLinear, input.tex);
    float4 texSpecColor = texSpecular.Sample(samLinear, input.tex);
    float4 texOpacColor = texOpacity.Sample(samLinear, input.tex);
    
    // emissive
    float4 emissive = texEmissive.Sample(samLinear, input.tex);
    
    // normal
    float3 norm = normalize(input.norm);
    float3 tan = normalize(input.tan);
    float3 binorm = normalize(input.binorm);
    float3x3 tbnMatrix = float3x3(tan, binorm, norm);
    
    float3 texNorm = DecodeNormal(texNormColor.rgb);
    float3 worldNorm = mul(texNorm, tbnMatrix);
    
    // ambient
    float4 ambient = texDiffColor * texOpacColor * materialAmbient * ambientLightColor;
    
    // diffuse
    float diffuseScalar = max(dot(worldNorm, -lightDir.xyz), 0.0f);
    float4 diffuse = texDiffColor * texOpacColor * lightColor * diffuseScalar;
    
    // specular
    float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
    float3 halfVector = normalize(-lightDir.xyz + viewDir);
    float specularScalar = max(dot(worldNorm, halfVector), 0.0f) * step(0.000001f, max(dot(norm, -lightDir.xyz), 0.0f));
    float4 specular = texSpecColor * materialSpecular * lightColor * pow(specularScalar, shininess.x);
    
    float4 finalColor = saturate(diffuse + ambient + specular + emissive);
    
    return finalColor;
}
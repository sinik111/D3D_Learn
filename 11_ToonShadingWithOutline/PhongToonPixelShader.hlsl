#include "Shared.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
    // normal
    float3 norm = normalize(input.norm);
    float3 tan = normalize(input.tan);
    float3 binorm = normalize(input.binorm);
    float3x3 tbnMatrix = float3x3(tan, binorm, norm);
    
    float3 texNorm = DecodeNormal(texNormColor.rgb);
    float3 worldNorm = normalize(mul(texNorm, tbnMatrix));
    
    // ambient
    float4 ambient = texDiffColor * materialAmbient * ambientLightColor;
    
    // diffuse
    float diffuseScalar = max(dot(worldNorm, -lightDir.xyz), 0.0f);
    float4 diffuse = texDiffColor * lightColor * diffuseScalar;
    
    // specular
    float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
    float3 halfVector = normalize(-lightDir.xyz + viewDir);
    float specularScalar = max(dot(worldNorm, halfVector), 0.0f) * step(0.000001f, max(dot(norm, -lightDir.xyz), 0.0f));
    float4 specular = texSpecColor * materialSpecular * lightColor * pow(specularScalar, shininess.x);
    
    // emissive
    float4 emissive = texEmissive.Sample(samLinear, input.tex);
    
    float4 finalColor = saturate(diffuse + ambient + specular + emissive);
    finalColor.a = texOpacColor.a;
    
    return finalColor;
}
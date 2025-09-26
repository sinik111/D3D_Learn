#include "Shared.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
    float4 texColor = texDiffuse.Sample(samLinear, input.tex);
    
    float3 norm = normalize(input.norm);
    float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
    
    // ambient
    float4 ambient = texColor * materialAmbient * ambientLightColor;
    
    // diffuse
    float diffuseScalar = max(dot(-lightDir.xyz, norm), 0.0f);
    float4 diffuse = texColor * lightColor * diffuseScalar;
    
    // specular
    float3 halfVector = normalize(-lightDir.xyz + viewDir);
    float specularScalar = max(dot(halfVector, norm), 0.0f) * step(0.0000001f, diffuseScalar);
    float4 specular = materialSpecular * lightColor * pow(specularScalar, shininess.x);
    
    float4 finalColor = saturate(diffuse + ambient + specular);
    
    return finalColor;
}
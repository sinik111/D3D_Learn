#include "Shared.hlsli"

float4 main(PS_INPUT input) : SV_Target
{
    float4 texColor = texDiffuse.Sample(samLinear, input.tex);
    
    float4 ambient = texColor * ambientColor;
    
    float diffuseScalar = max(dot((float3) -lightDir, input.norm), 0.0f);
    
    float4 diffuse = texColor * diffuseColor * diffuseScalar;
    
    float3 halfVector = normalize((float3) -lightDir + input.viewDir);
    
    float specularScalar = max(dot(halfVector, input.norm), 0.0f);
    
    float4 specular = specularColor * pow(specularScalar, (float) shininess);
    
    float4 finalColor = saturate(diffuse + ambient + specular);
    
    return finalColor;
}
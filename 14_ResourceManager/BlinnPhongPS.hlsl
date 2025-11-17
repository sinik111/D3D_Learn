#include "Shared.hlsli"
#include "TextureSampling.hlsli"
#include "ShadowMapping.hlsli"

float4 main(PS_INPUT_SHADOW input) : SV_Target
{    
    MaterialData materialData = SampleMaterialData(input);
        
    // shadow
    float shadowFactor = CalculateShadowFactor(input);
    
    // diffuse, ambient
    float diffuseIntensity = saturate(dot(materialData.norm, -g_lightDir)) * shadowFactor;
    float4 diffuse = materialData.diffuseColor * g_lightColor * diffuseIntensity;
    float4 ambient = materialData.diffuseColor * g_ambientLightColor;
    
    // specular
    float3 viewDir = normalize(g_cameraPos - input.worldPos);
    float3 halfVector = normalize(-g_lightDir + viewDir);
    float nDotH = saturate(dot(materialData.norm, halfVector));
    float specularIntensity = diffuseIntensity > 0.0f ? pow(nDotH, materialData.shininess) : 0.0f;
    
    float4 specular = materialData.specularColor * g_lightColor * specularIntensity;
    
    float4 finalColor = saturate(diffuse + ambient + specular + materialData.emissiveColor);
    
#ifdef ALPHA_BLEND
    finalColor.a = materialData.opacity;
#endif //ALPHA_BLEND
    
    return finalColor;
}
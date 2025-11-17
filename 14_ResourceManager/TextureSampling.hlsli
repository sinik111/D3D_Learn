#ifndef TEXTURE_SAMPLING_HLSLI
#define TEXTURE_SAMPLING_HLSLI

#include "Shared.hlsli"

#ifdef USE_DIFFUSE_MAP
Texture2D g_texDiffuse : register(t0);
#endif //USE_DIFFUSE_MAP

#ifdef USE_NORMAL_MAP
Texture2D g_texNormal : register(t1);
#endif //USE_NORMAL_MAP

#ifdef USE_SPECULAR_MAP
Texture2D g_texSpecular : register(t2);
#endif //USE_SPECULAR_MAP

#ifdef USE_EMISSIVE_MAP
Texture2D g_texEmissive : register(t3);
#endif //USE_EMISSIVE_MAP

#ifdef USE_OPACITY_MAP
Texture2D g_texOpacity : register(t4);
#endif //USE_OPACITY_MAP

struct MaterialData
{
    float4 diffuseColor;
    float4 ambientColor;
    float4 specularColor;
    float4 emissiveColor;
    float3 norm;
    float opacity;
    float shininess;
};

MaterialData SampleMaterialData(PS_INPUT_SHADOW input)
{
    MaterialData data;
    
    data.opacity = 1.0f;
    
    #ifdef USE_OPACITY_MAP
        data.opacity = g_texOpacity.Sample(g_samLinear, input.tex);

        #ifdef ALPHA_TEST
            clip(data.opacity - 0.5f);
        #endif //ALPHA_TEST
    #endif //USE_OPACITY_MAP
    
    #ifdef HAS_OPACITY_FACTOR
    data.opacity *= g_opacity;
    #endif //HAS_OPACITY_FACTOR
    
    data.norm = input.norm;
    #ifdef USE_NORMAL_MAP
        float3x3 tbnMatrix = float3x3(
            normalize(input.tan),
            normalize(input.binorm),
            normalize(input.norm));
    
        float3 texNorm = DecodeNormal(g_texNormal.Sample(g_samLinear, input.tex).rgb);
        data.norm = normalize(mul(texNorm, tbnMatrix));
    #endif //USE_NORMAL_MAP
    
    data.diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    #ifdef USE_DIFFUSE_MAP
        data.diffuseColor = g_texDiffuse.Sample(g_samLinear, input.tex);
    #endif //USE_DIFFUSE_MAP
    
    #ifdef HAS_DIFFUSE_COLOR
        data.diffuseColor *= g_materialDiffuse;
    #endif //HAS_DIFFUSE_COLOR
    
    data.ambientColor = data.diffuseColor;
    
    #ifdef HAS_AMBIENT_COLOR
        data.ambientColor *= g_materialAmbient;
    #endif //HAS_AMBIENT_COLOR
    
    data.specularColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    #ifdef USE_SPECULAR_MAP
        data.specularColor = g_texSpecular.Sample(g_samLinear, input.tex);
    #endif //USE_SPECULAR_MAP
    
    #ifdef HAS_SPECULAR_COLOR
        data.specularColor *= g_materialSpecular;
    #endif //HAS_SPECULAR_COLOR
    
    data.emissiveColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    #ifdef USE_EMISSIVE_MAP
         data.emissiveColor = g_texEmissive.Sample(g_samLinear, input.tex);
    #endif //USE_EMISSIVE_MAP
    
    #ifdef HAS_EMISSIVE_COLOR
         data.emissiveColor = g_materialEmissive;
    #endif //HAS_EMISSIVE_COLOR
    
    data.shininess = 64.0f;
    #ifdef HAS_SHININESS_FACTOR
         data.shininess = g_shininess;
    #endif //HAS_SHININESS_FACTOR
    
    return data;
}

#endif //TEXTURE_SAMPLING_HLSLI
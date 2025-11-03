cbuffer ConstantBuffer : register(b0)
{
    matrix g_world;
    matrix g_view;
    matrix g_projection;
    
    float4 g_materialDiffuse;
    float4 g_materialAmbient;
    float4 g_materialSpecular;
    float4 g_materialEmissive;
    
    float3 g_lightDirection;
    float __pad1;
    float4 g_lightColor;
    float4 g_ambientLightColor;
    float g_shininess;
    
    float g_emissivePosition;
    float g_outlineThickness;
    float g_elapsedTime;
    float g_outlineFrequency;
    float g_outlineDensity;
    float __pad2[2];
}

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 viewPos : TEXCOORD2;
    float3 viewLightDir : TEXCOORD3;
    float3 localPos : TEXCOORD4;
};

struct VS_INPUT_OUTLINE
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct PS_INPUT_OUTLINE
{
    float4 position : SV_Position;
    float4 color : COLOR;
};
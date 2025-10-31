cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    
    float4 materialAmbient;
    float4 materialSpecular;
    
    float4 cameraPos;
    float4 lightDir;
    float4 lightColor;
    float4 ambientLightColor;
    float shininess;
    float3 __pad1;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
    float3 norm : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float3 worldPos : TEXCOORD4;
};
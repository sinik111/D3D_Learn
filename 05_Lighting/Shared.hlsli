
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix normalMatrix;
    
    float4 lightDir;
    float4 lightColor;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 norm : TEXCOORD0;
};
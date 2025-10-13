SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix normalMatrix;
    
    float4 materialAmbient;
    float4 materialSpecular;
    
    float4 cameraPos;
    float4 lightDir;
    float4 lightColor;
    float4 ambientLightColor;
    float4 shininess;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float3 binorm : BINORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 tex : TEXCOORD0;
    float3 norm : TEXCOORD1;
    float3 tan : TEXCOORD2;
    float3 binorm : TEXCOORD3;
    float3 worldPos : TEXCOORD4;
};

struct VS_SKYBOX_INPUT
{
    float3 pos : POSITION;
};

struct PS_SKYBOX_INPUT
{
    float4 pos : SV_Position;
    float3 originPos : TEXCOORD0;
};

struct INS_VS_INPUT
{
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
    float3 norm : NORMAL;
    float3 tan : TANGENT;
    float3 binorm : BINORMAL;
    
    float4 world0 : WORLD0;
    float4 world1 : WORLD1;
    float4 world2 : WORLD2;
    float4 world3 : WORLD3;
};

float3 EncodeNormal(float3 n)
{
    return n * 0.5f + 0.5f;
}

float3 DecodeNormal(float3 n)
{
    return n * 2.0f - 1.0f;
}
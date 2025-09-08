
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
}

struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

PS_INPUT main(float4 pos : POSITION, float4 color : COLOR)
{
    PS_INPUT output = (PS_INPUT)0;
    //output.pos = pos;
    output.pos = mul(pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = color;
    
    return output;
}
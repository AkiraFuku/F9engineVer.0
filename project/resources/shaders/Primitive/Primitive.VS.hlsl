
struct VS_INPUT
{
    float4 pos : POSITION0;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};


VS_OUTPUT main(VS_INPUT input) 
{
    
    VS_OUTPUT output;
    output.pos = input.pos;
    return output;
    
}
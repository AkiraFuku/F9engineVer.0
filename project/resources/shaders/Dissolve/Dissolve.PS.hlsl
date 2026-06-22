#include "../CopyImage/FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture: register(t1);
SamplerState gSampler : register(s0);


struct DissolveParameter
{
    float threshold;
};
ConstantBuffer<DissolveParameter> gDissolveParam : register(b0);
//: register(b1);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
PixelShaderOutput main(
VertexShaderOutput input)
{
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    if (mask <= gDissolveParam.threshold)
    {
        discard;
    }
        
    PixelShaderOutput output;
    output.color=gTexture.Sample(gSampler, input.texcoord);
    
    return output;
}
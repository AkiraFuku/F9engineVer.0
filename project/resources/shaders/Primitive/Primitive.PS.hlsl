

struct Material
{
    float4 coler;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PS_OUTPUT
{
    float4 color : SV_Target0;
};

PS_OUTPUT main() 
{
    
    PS_OUTPUT output;
    output.color = gMaterial.coler;
    return output;
    
}
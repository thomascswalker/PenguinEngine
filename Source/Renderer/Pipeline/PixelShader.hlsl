struct PSInput
{
    float4 position: SV_POSITION;
};

struct PSOutput
{
    float4 color: SV_Target0;
};

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput)0;
    output.color = float4(1.0,0.0,0.0, 1.0);
    return output;
}
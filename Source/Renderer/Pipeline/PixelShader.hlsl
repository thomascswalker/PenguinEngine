struct PS_INPUT
{
    float4 position: SV_POSITION;
    float3 normal: NORMAL;
    float2 tex: TEXCOORD0;
    float3 direction: TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 finalColor = 0;
    float4 rgb = dot(input.direction, input.normal);
    finalColor = finalColor + rgb;
    return finalColor;
}
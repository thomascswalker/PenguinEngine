struct PS_INPUT
{
    float4 position: SV_POSITION;
    float3 normal: NORMAL;
    float3 cameraDirection: TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 rgb = dot(-input.cameraDirection, input.normal);
    float4 finalColor = float4(rgb, 1);
    return finalColor;
}
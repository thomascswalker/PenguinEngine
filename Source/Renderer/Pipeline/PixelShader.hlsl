struct PS_INPUT
{
    float4 position: SV_POSITION;
    float3 normal: NORMAL;
    float2 tex: TEXCOORD0;
    float3 direction: TEXCOORD1;
};

Texture2D _texture;
SamplerState _sampler;

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 diffuse = _texture.Sample(_sampler, input.tex);
    float4 lighting = dot(input.direction, input.normal);
    float4 finalColor = diffuse * lighting;
    return finalColor;
}
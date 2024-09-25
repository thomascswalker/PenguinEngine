// https://graphicsprogramming.github.io/learnd3d11/1-introduction/1-1-getting-started/1-1-3-hello-triangle/#vertex-shader

cbuffer constants : register(b0) 
{
    matrix mvp;
    matrix model;
    matrix view;
    matrix projection;
    float3 direction;
};

struct VS_Input
{
    float3 position: SV_POSITION;
    float3 normal: NORMAL;
};

struct VS_OUTPUT
{
    float4 position: SV_POSITION;
    float3 normal: NORMAL;
    float3 direction: TEXCOORD0;
};

VS_OUTPUT main(VS_Input input)
{
    VS_OUTPUT output;

    // World to screen
    output.position = mul(float4(input.position, 1.0f), mvp);

    // Tangent to Object normal
    output.normal = input.normal;

    // Camera direction
    output.direction = direction;

    return output;
}

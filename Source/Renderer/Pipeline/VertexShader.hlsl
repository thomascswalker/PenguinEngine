// https://graphicsprogramming.github.io/learnd3d11/1-introduction/1-1-getting-started/1-1-3-hello-triangle/#vertex-shader

// Camera
cbuffer CameraBuffer : register(b0) 
{
    matrix viewProjection;
    float4 cameraDirection;
};

// Model
cbuffer ModelBuffer : register(b1)
{
    matrix model;
};

struct VS_Input
{
    float3 position: SV_POSITION;
    float3 normal: NORMAL;
    float2 tex: TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position: SV_POSITION;
    float3 normal: NORMAL;
    float2 tex: TEXCOORD0;
    float3 direction: TEXCOORD1;
};

VS_OUTPUT main(VS_Input input)
{
    VS_OUTPUT output;

    // World to screen
    matrix mvp = mul(model, viewProjection);
    output.position = mul(float4(input.position, 1.0f), mvp);

    // Tangent to Object normal
    output.normal = mul(float4(input.normal, 1.0f), model).xyz;

    output.tex = input.tex;

    // Camera direction
    output.direction = cameraDirection;

    return output;
}

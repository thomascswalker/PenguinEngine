// https://graphicsprogramming.github.io/learnd3d11/1-introduction/1-1-getting-started/1-1-3-hello-triangle/#vertex-shader

struct VSInput
{
    float3 position: SV_POSITION;
};

struct VSOutput
{
    float4 position: SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.position = float4(input.position, 1.0);
    return output;
}
import CommonTypes;

[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 world2View;

    float3* positions;
    float4* colors;
};

struct VSOut
{
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOut vsMain(in uint vertexId : SV_VertexID)
{
    VSOut result;
    float3 position = positions[vertexId];
    float4 color = colors[vertexId];

    result.position = mul(world2View, float4(position, 1.f));
    result.color = color.xyz;

    return result;
}

[shader("pixel")]
float4 psMain(in VSOut vsOut) : SV_Target0
{
    return float4(vsOut.color, 1.f);
}

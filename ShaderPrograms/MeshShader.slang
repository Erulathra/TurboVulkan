import CommonTypes;

[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 world2View;
    VertexWithColor* vertices;
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
    VertexWithColor vertex = vertices[vertexId];

    result.position = mul(world2View, float4(vertex.position, 1.f));
    result.color = vertex.color.xyz;

    return result;
}

[shader("pixel")]
float4 psMain(in VSOut vsOut) : SV_Target0
{
    return float4(vsOut.color, 1.f);
}

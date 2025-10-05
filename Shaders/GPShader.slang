import Math;

[[vk::binding(0, 0)]]
StructuredBuffer<float3, ScalarDataLayout> positions;

[[vk::binding(1, 0)]]
StructuredBuffer<uint> indices;

[[vk::push_constant]]
cbuffer pushConstants
{
    float time;
};

struct VSIn
{
    uint vertexId : SV_VertexID;
}

struct VSOut
{
    float4 position : SV_Position;
}

struct PSOut
{
    float4 color : SV_Target0;
}

[shader("vertex")]
void vsMain(in VSIn vsIn, out VSOut vsOut)
{
    // const float size = lerp(0.2f, 1.f, Sin0To1(time * 3));
    const uint indice = indices[vsIn.vertexId];
    vsOut.position = float4(positions[indice], 1.f);
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    psOut.color = float4(0.f, 0.f, 1.f, 1.f);
}

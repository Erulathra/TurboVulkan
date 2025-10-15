import Math;

[[vk::binding(0, 0)]]
StructuredBuffer<float3, ScalarDataLayout> positions;

[[vk::push_constant]]
cbuffer pushConstants
{
    float4x4 modelToProj;
    float3* positionBuffer;
};

struct VSIn
{
    uint vertexId : SV_VertexID;
}

struct VSOut
{
    float4 position : SV_Position;
    float3 color;
}

struct PSOut
{
    float4 color : SV_Target0;
}

[shader("vertex")]
void vsMain(in VSIn vsIn, out VSOut vsOut)
{
    vsOut.position = mul(float4(positions[vsIn.vertexId], 1.f), modelToProj);
    // vsOut.position = mul(float4(positionBuffer[vsIn.vertexId], 1.f), modelToProj);
    vsOut.color = float3(1.f, 0.f, 1.f);
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    psOut.color = float4(vsOut.color, 1.f);
}

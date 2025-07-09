
static const float3 positions[3] = {
    float3(1.f, 1.f, 0.f),
    float3(-1.f, 1.f, 0.f),
    float3(0.f, -1.f, 0.f)
};

static const float3 colors[3] = {
    float3(1.f, 0.f, 0.f),
    float3(0.f, 1.f, 0.f),
    float3(0.f, 0.f, 1.f)
};

struct VSIn
{
    uint position : SV_VertexID;
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
    vsOut.position = float4(positions[vsIn.position] * 0.5, 1.f);
    vsOut.color = colors[vsIn.position];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    psOut.color = float4(vsOut.color, 1.f);
}

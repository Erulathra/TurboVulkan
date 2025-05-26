
const static float2 Positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

const static float4 Colors[3] = float4[](
    float4(1.f, 0.f, 0.f, 1.f),
    float4(0.f, 1.f, 0.f, 1.f),
    float4(0.f, 0.f, 1.f, 1.f)
);

struct VSOut
{
    float4 Position : SV_Position;
    float4 Color;
};


[shader("vertex")]
VSOut VertexMain(uint VertexId : SV_VertexID)
{
    VSOut Result;
    Result.Position = float4(Positions[VertexId % 3], 1.f);
    Result.Color = Colors[VertexId % 3];

    return Result;
}

[shader("fragment")]
float4 PixelMain(VSOut VSin) : SV_Target0
{
    return VSin.Color;
}


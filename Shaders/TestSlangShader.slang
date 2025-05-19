[shader("vertex")]
float4 main(float3 Position : POSITION) : SV_Position
 {
    return float4(Position, 1.f);
}
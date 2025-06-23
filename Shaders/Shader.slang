[[vk::binding(0, 0)]]
RWTexture2D<float4> inOutTexture;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint3 groupThreadId : SV_GroupThreadID)
{
    uint2 imageSize;
    inOutTexture.GetDimensions(imageSize.x, imageSize.y);

    if (any(threadId.xy >= imageSize))
        return;

    const float2 uv = float2(threadId.xy) / float2(imageSize);
    float3 outColor = float3(uv.x, uv.y, 0.0f);
    outColor *= all(groupThreadId <= 1) ? 1.0f : 0.5f;

    inOutTexture[threadId.xy] = float4(uv.x, uv.y, 0.0f, 1.0f);
}


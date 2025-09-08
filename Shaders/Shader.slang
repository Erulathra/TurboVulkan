#include "MathConstants.slang"

import Math;

[[vk::binding(0, 0)]]
RWTexture2D<float4> inOutTexture;

[[vk::push_constant]]
cbuffer pushConstants
{
    float4 colorOne;
    float4 colorTwo;

    float time;
};

static const float2 triangleVertices[3] = {
    float2(0.5f, 0.5f),
    float2(0.2f, 0.8f),
    float2(0.8f, 0.8f)
};

bool OnRightSide(float2 a, float2 b, float2 p)
{
    const float2 ap = p - a;
    const float2 ab = b - a;
    const float2 abPerp = float2(ab.y, -ab.x);
    return dot(ap, abPerp) >= 0.0f;
}

bool PointInTriangle(float2 a, float2 b, float2 c, float2 p)
{
    const bool side1 = OnRightSide(a, b, p);
    const bool side2 = OnRightSide(b, c, p);
    const bool side3 = OnRightSide(c, a, p);

    return side1 && side2 && side3;
}

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

    float angle = time * M_PI;

    outColor *= lerp(0.1f, 1.f, PointInTriangle(
        RotateUV(triangleVertices[0], angle),
        RotateUV(triangleVertices[1], angle),
        RotateUV(triangleVertices[2], angle),
        uv
    ) ? 1.0f : 0.0f);

    inOutTexture[threadId.xy] = float4(outColor.x, outColor.y, 0.0f, 1.0f);
}


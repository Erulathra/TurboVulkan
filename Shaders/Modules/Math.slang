module Math;

import MathTypes;

#include "MathConstants.slang"

public float2 RotateUV(float2 uv, float rotation, float2 mid)
{
    return float2(
        cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) + mid.x,
        cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) + mid.y);
}

public float2 RotateUV(float2 uv, float rotation, float mid)
{
    return RotateUV(uv, rotation, float2(mid, mid));
}

public float2 RotateUV(float2 uv, float rotation)
{
    return RotateUV(uv, rotation, 0.5f);
}

public float LenghtSquared(float2 v)
{
    return dot(v, v);
}

public float LenghtSquared(float3 v)
{
    return dot(v, v);
}

public float LenghtSquared(float4 v)
{
    return dot(v, v);
}

public float InvLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}

public float Remap(float origFrom, float origTo, float targetFrom, float targetTo, float value)
{
    float rel = InvLerp(origFrom, origTo, value);
    return lerp(targetFrom, targetTo, rel);
}

public float Sin0To1(float x)
{
    return sin(x) * 0.5f + 0.5f;
}

public float3 HSVToRGB(float3 hsv)
{
    // https://gist.github.com/iUltimateLP/5129149bf82757b31542
    const float4 k = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    const float3 p = abs(frac(hsv.xxx + k.xyz) * 6.0 - k.www);

    return hsv.z * lerp(k.xxx, saturate(p - k.xxx), hsv.y);
}

public float3 ExtractScaleSquared(float4x4 transform)
{
    return float3(
        LenghtSquared(transform[0].xyz),
        LenghtSquared(transform[1].xyz),
        LenghtSquared(transform[2].xyz)
    );
}

public float SignedDistanceToPlane(float3 point, FPlane plane)
{
    return dot(plane.mNormal, point) - plane.mDistance;
}

public FSphere TransformSphere(float4x4 transform, FSphere sphere)
{
    const float3 newCenter = mul(float4(sphere.mCenter, 1.0f), transform).xyz;

    const float3 scaleSquared = ExtractScaleSquared(transform);
    const float maxScaleSquared = max3(scaleSquared.x, scaleSquared.y, scaleSquared.z);
    const float newRadius = sphere.mRadius * sqrt(maxScaleSquared);

    return FSphere(newCenter, newRadius);
}

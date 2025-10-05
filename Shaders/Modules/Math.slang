module Math;

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

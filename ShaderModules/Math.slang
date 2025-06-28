module Math;

#include "MathConstants.slang"

public float2 RotateUV(float2 uv, float rotation, float2 mid)
{
    return float2(
      cos(rotation) * (uv.x - mid.x) + sin(rotation) * (uv.y - mid.y) + mid.x,
      cos(rotation) * (uv.y - mid.y) - sin(rotation) * (uv.x - mid.x) + mid.y
    );
}

public float2 RotateUV(float2 uv, float rotation, float mid)
{
    return RotateUV(uv, rotation, float2(mid, mid));
}

public float2 RotateUV(float2 uv, float rotation)
{
    return RotateUV(uv, rotation, 0.5f);
}

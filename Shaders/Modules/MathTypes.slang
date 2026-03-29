module Math;

#include "MathConstants.slang"

public struct FPlane
{
    public float3 mNormal;
    public float mDistance;
}

public struct FSphere
{
    public float3 mCenter;
    public float mRadius;
}

public struct FFrustum
{
    // 0: near, 1: far, 2: left, 3: right, 4: top, 5: bottom
    public FPlane mPlanes[6];
}

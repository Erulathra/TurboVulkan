module ViewData;
import MathTypes;

public struct FViewData
{
    public float4x4 mProjectionMatrix;
    public float4x4 mViewMatrix;

    public float4x4 mWorldToProjection;
    public float3 mViewPosition;

    public double mTime;
    public double mWorldTime;
    public double mDeltaTime;
    public uint mFrameIndex;

    // In world space
    public FFrustum mViewFrustum;
};

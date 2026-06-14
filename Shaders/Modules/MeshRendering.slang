module MeshRendering;

import MathTypes;
import ViewData;

public enum ECameraType
{
    Perspective,
    Orthographic
}

public struct FMaterialInstance
{
    public uint mDiffuseTexture;
    public uint mDiffuseSampler;
};

public struct FBounds
{
    public float3 mMin;
    public float mRadius;
    public float3 mMax;
    public float mRadiusSquared;
}

public struct FMeshData
{
    public const Ptr<float> mIndexBuffer;
    public const Ptr<float3> mPositionBuffer;
    public const Ptr<float3> mNormalBuffer;
    public const Ptr<float4> mTangentBuffer;
    public const Ptr<float2> mUVBuffer;

    public const uint mVertexCount;
    public const uint mMeshIndex;
}

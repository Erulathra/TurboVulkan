module CommonTypes;

import MathTypes;

public enum ECameraType
{
    Perspective,
    Orthographic
}

public struct FViewData
{
    public float4x4 mProjectionMatrix;
    public float4x4 mViewMatrix;

    public float4x4 mWorldToProjection;

    public double mTime;
    public double mWorldTime;
    public double mDeltaTime;
    public uint mFrameIndex;

    // In world space
    public FFrustum mViewFrustum;
};

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
    public const Ptr<float2> mUVBuffer;

    public const uint mVertexCount;
    public const uint mMeshIndex;
}

public struct FSceneData
{
    public float4 mDummyData;
}

public struct FCommonPushConstants
{
    public float4x4 mModelToProj;
    public float4x4 mModelToView;
    public float3x3 mNormalModelToView;

    public const Ptr<FViewData> mViewData;
    public const Ptr<void> mMaterialData;
    public const Ptr<void> mMaterialInstance;
    public const Ptr<FMeshData> mMeshData;
    public const Ptr<FSceneData> mSceneData;
};

public struct FIndirectDrawData
{
    public float4x4 mModelToProj;
    public float4x4 mModelToView;
    public float4x4 mModelToWorld;
    public float3x3 mNormalModelToView;

    public const Ptr<void> mMaterialData;
    public const Ptr<void> mMaterialInstance;
    public const Ptr<FMeshData> mMeshData;
};

public struct FIndirectPushConstants
{
    public const Ptr<FViewData> mViewData;
    public const Ptr<FSceneData> mSceneData;

    public const Ptr<FIndirectDrawData> mDrawData;
};

public struct FDrawIndirectCommand
{
    public uint mVertexCount;
    public uint mInstanceCount;
    public uint mFirstVertex;
    public uint mFirstInstance;
};

public struct FDrawIndexedIndirectCommand
{
    public uint mIndexCount;
    public uint mInstanceCount;
    public uint mFirstIndex;
    public int mVertexOffset;
    public uint mFirstInstance;
};

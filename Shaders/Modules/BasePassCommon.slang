module BasePassCommon;

import MathTypes;
import ShadingCommon;
import MeshRendering;

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
    public float3 mViewPosition;

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

public struct FSceneData
{
    public uint mNumLights;
    uint _PADDING[3];
}

public struct FIndirectPushConstants
{
    public const Ptr<FViewData> mViewData;
    public const Ptr<FSceneData> mSceneData;
    public const Ptr<FLight> mLightData;

    public const Ptr<FIndirectDrawData> mDrawData;
};

public struct FDrawIndexedIndirectCommand
{
    public uint mIndexCount;
    public uint mInstanceCount;
    public uint mFirstIndex;
    public int mVertexOffset;
    public uint mFirstInstance;
};

public struct FIndirectDrawData
{
    public float4x4 mModelToProj;
    public float4x4 mModelToView;
    public float4x4 mModelToWorld;
    public float3x3 mNormalModelToWorld;

    public const Ptr<void> mMaterialData;
    public const Ptr<void> mMaterialInstance;
    public const Ptr<FMeshData> mMeshData;
};

public struct FDrawIndirectCommand
{
    public uint mVertexCount;
    public uint mInstanceCount;
    public uint mFirstVertex;
    public uint mFirstInstance;
};

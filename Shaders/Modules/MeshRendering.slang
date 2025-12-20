module CommonTypes;

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

    public double time;
    public double worldTime;
    public double deltaTime;
    public uint frameIndex;
};

public struct FMaterialInstance
{
    public uint diffuseTexture;
    public uint diffuseSampler;
};

public struct FMeshData
{
    public const Ptr<float3> positionBuffer;
    public const Ptr<float3> normalBuffer;
    public const Ptr<float2> uvBuffer;
    public const Ptr<float4> colorBuffer;
}

public struct FSceneData
{
    public float4 dummyData;
}

public struct FCommonPushConstants
{
    public float4x4 modelToProj;
    public float3x3 normalModelToWorld;

    public const Ptr<FViewData> viewData;
    public const Ptr<void> materialInstance;
    public const Ptr<FMeshData> meshData;
    public const Ptr<FSceneData> sceneData;
};

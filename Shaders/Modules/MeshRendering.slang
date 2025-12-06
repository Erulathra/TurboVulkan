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
    public float3* positionBuffer;
    public float3* normalBuffer;
    public float2* uvBuffer;
    public float4* colorBuffer;
}

public struct FSceneData
{
    public float4 dummyData;
}

public struct FCommonPushConstants
{
    public float4x4 modelToProj;
    public float3x3 normalModelToWorld;

    public FViewData* viewData;
    public FMaterialInstance* materialInstance;
    public FMeshData* meshData;
    public FSceneData* sceneData;
};

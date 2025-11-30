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

    float time;
    float worldTime;
    float deltaTime;
    float frameIndex;
};

public struct FMaterialInstance
{
    public uint diffuseTexture;
    public uint diffuseSampler;

    uint PADDING[3];
};

public struct FMeshData
{
    public float3* positionBuffer;
    public float3* normalBuffer;
    public float2* uvBuffer;
    public float4* colorBuffer;
}

public struct FCommonPushConstants
{
    public float4x4 modelToProj;
    public float3x3 invModelToProj;

    public FViewData* viewData;
    public FMaterialInstance* materialInstance;
    public FMeshData* meshData;
};

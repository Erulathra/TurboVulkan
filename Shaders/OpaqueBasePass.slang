#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;

[[vk::push_constant]]
FCommonPushConstants pc;

struct MaterialData
{
    uint mBaseColorSampler;
    uint mMetalicRoughnessSampler;
    uint mNormalSampler;
}

struct InstanceData
{
    uint mBaseColorTexture;
    uint mMetalicRoughnessTexture;
    uint mNormalTexture;

    float mNormalScale;
    float4 mBaseColorFactor;
    float mMetalicFactor;
    float mRoughnessFactor;
}

struct VSOut
{
    float4 mPosition : SV_Position;
    float3 mViewPosition;
    float3 mViewNormal;
    float2 mUV;
}

struct PSOut
{
    float4 mColor : SV_Target0;
}

#define ENABLE_VERTEX_PULLING 0

[shader("vertex")]
void vsMain(in uint indexId : SV_VertexID, out VSOut vsOut)
{
    FMeshData* mesh = pc.mMeshData;

#if ENABLE_VERTEX_PULLING
	const uint vertexId = asuint(mesh.mIndexBuffer[indexId]);
#else
	const uint vertexId = indexId;
#endif
	const float4 modelPosition = float4(mesh.mPositionBuffer[vertexId], 1.f);

    vsOut.mPosition = mul(modelPosition, pc.mModelToProj);
    vsOut.mViewPosition = mul(modelPosition, pc.mModelToView).xyz;
    vsOut.mViewNormal = mul(mesh.mNormalBuffer[vertexId], pc.mNormalModelToView);
    vsOut.mUV = mesh.mUVBuffer[vertexId];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const Ptr<InstanceData> instance = Ptr<InstanceData>(pc.mMaterialInstance);
    const Ptr<MaterialData> material = Ptr<MaterialData>(pc.mMaterialData);

    Texture2D<float3> texture = texturePool[instance.mBaseColorTexture];
    SamplerState sampler = samplerPool[material.mBaseColorSampler];

    const float3 baseColor = instance.mBaseColorFactor.rgb * texture.Sample(sampler, vsOut.mUV);

    psOut.mColor = float4(baseColor, 1.f);
}

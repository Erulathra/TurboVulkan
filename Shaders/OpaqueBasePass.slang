#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;

[[vk::push_constant]]
FIndirectPushConstants pc;

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

	uint mDrawId;
}

struct PSOut
{
    float4 mColor : SV_Target0;
}

[shader("vertex")]
void vsMain(
    in uint indexId : SV_VertexID,
	in uint drawId : SV_StartInstanceLocation,
    out VSOut vsOut
)
{
	const FIndirectDrawData data = pc.mDrawData[drawId];
    const Ptr<FMeshData> mesh = data.mMeshData;

	const uint vertexId = asuint(mesh.mIndexBuffer[indexId]);
	const float4 modelPosition = float4(mesh.mPositionBuffer[vertexId], 1.f);

    vsOut.mPosition = mul(modelPosition, data.mModelToProj);
    vsOut.mViewPosition = mul(modelPosition, data.mModelToView).xyz;
    vsOut.mViewNormal = mul(mesh.mNormalBuffer[vertexId], data.mNormalModelToView);
    vsOut.mUV = mesh.mUVBuffer[vertexId];
	vsOut.mDrawId = drawId;
}

[shader("pixel")]
void psMain(
    in VSOut vsOut,
    out PSOut psOut
)
{
	const FIndirectDrawData data = pc.mDrawData[vsOut.mDrawId];

    const Ptr<InstanceData> instance = Ptr<InstanceData>(data.mMaterialInstance);
    const Ptr<MaterialData> material = Ptr<MaterialData>(data.mMaterialData);

    Texture2D<float3> texture = texturePool[instance.mBaseColorTexture];
    SamplerState sampler = samplerPool[material.mBaseColorSampler];

    const float3 baseColor = instance.mBaseColorFactor.rgb * texture.Sample(sampler, vsOut.mUV);

    psOut.mColor = float4(baseColor, 1.f);
}

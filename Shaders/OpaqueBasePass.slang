#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;

[[vk::push_constant]]
FCommonPushConstants pc;

struct MaterialData
{
    uint baseColorSampler;
    uint metalicRoughnessSampler;
    uint normalSampler;
}

struct InstanceData
{
    uint baseColorTexture;
    uint metalicRoughnessTexture;
    uint normalTexture;

    float normalScale;
    float4 baseColorFactor;
    float metalicFactor;
    float roughnessFactor;
}

struct VSOut
{
    float4 position : SV_Position;
    float3 viewPosition;
    float3 viewNormal;
    float2 uv;
}

struct PSOut
{
    float4 color : SV_Target0;
}

[shader("vertex")]
void vsMain(in uint vertexId : SV_VertexID, out VSOut vsOut)
{
    FMeshData* mesh = pc.meshData;

	const float4 modelPosition = float4(mesh.positionBuffer[vertexId], 1.f);

    vsOut.position = mul(modelPosition, pc.modelToProj);
    vsOut.viewPosition = mul(modelPosition, pc.modelToView).xyz;
    vsOut.viewNormal = mul(mesh.normalBuffer[vertexId], pc.normalModelToView);
    vsOut.uv = mesh.uvBuffer[vertexId];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const Ptr<InstanceData> instance = Ptr<InstanceData>(pc.materialInstance);
    const Ptr<MaterialData> material = Ptr<MaterialData>(pc.materialData);

    Texture2D<float3> texture = texturePool[instance.baseColorTexture];
    SamplerState sampler = samplerPool[material.baseColorSampler];

    const float3 baseColor = instance.baseColorFactor.rgb * texture.Sample(sampler, vsOut.uv);

    psOut.color = float4(baseColor, 1.f);
}

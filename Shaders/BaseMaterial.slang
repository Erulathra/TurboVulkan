#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;

[[vk::push_constant]]
FCommonPushConstants pc;

struct MaterialData
{
    uint texture;
	uint sampler;
}

struct InstanceData
{
    float3 color;
	float emissive;
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

    const float3 sunWorldPos = float3(0.f);
    const float3 sunPos = mul(float4(sunWorldPos, 1.f), pc.viewData.mViewMatrix).xyz;

    const float ambientLight = 0.01f + instance.emissive;

    const float3 sunDir = normalize(vsOut.viewPosition - sunPos);

    float lightIntensity = ambientLight + max(0.f, dot(vsOut.viewNormal, -sunDir));
    lightIntensity = saturate(lightIntensity);

    Texture2D<float3> texture = texturePool[material.texture];
    SamplerState sampler = samplerPool[material.sampler];

    const float3 baseColor = instance.color * texture.Sample(sampler, vsOut.uv);
    const float3 colorLightPass = baseColor * lightIntensity;

    psOut.color = float4(colorLightPass, 1.f);
}

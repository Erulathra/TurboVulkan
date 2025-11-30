#include "Modules/Common.slang"

import Modules.Math;
import Modules.CommonTypes;

[[vk::push_constant]]
FCommonPushConstants pushConstants;

struct VSOut
{
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
}

struct PSOut
{
    float4 color : SV_Target0;
}

[shader("vertex")]
void vsMain(in uint vertexId : SV_VertexID, out VSOut vsOut)
{
    FMeshData* meshData = pushConstants.meshData;

    vsOut.position = mul(float4(meshData.positionBuffer[vertexId], 1.f), pushConstants.modelToProj);
    vsOut.normal = meshData.normalBuffer[vertexId];
    vsOut.uv = meshData.uvBuffer[vertexId];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const float3 sunDir = normalize(float3(-1.f, -1.f, -1.f));
    const float ambientLight = 0.1f;

    float lightIntensity = ambientLight + max(0.f, dot(vsOut.normal, -sunDir));
    lightIntensity = clamp(lightIntensity, 0.f, 1.f);

    uint diffuseTextureId = pushConstants.materialInstance.diffuseTexture;
    uint diffuseSamplerId = pushConstants.materialInstance.diffuseSampler;

    const float3 baseColor = texturePool[diffuseTextureId].Sample(samplerPool[diffuseSamplerId], vsOut.uv);
    const float3 colorLightPass = baseColor * lightIntensity;

    psOut.color = float4(baseColor, 1.f);
}

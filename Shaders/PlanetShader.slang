#include "Modules/Common.slang"

import Modules.Math;

[[vk::push_constant]]
cbuffer pushConstants
{
    float4x4 modelToProj;
    float3* positionBuffer;
    float3* normalBuffer;
    float2* uvBuffer;

    uint textureId;
    uint samplerId;
};

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
    vsOut.position = mul(float4(positionBuffer[vertexId], 1.f), modelToProj);
    vsOut.normal = normalBuffer[vertexId];
    vsOut.uv = uvBuffer[vertexId];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const float3 sunDir = normalize(float3(-1.f, -1.f, -1.f));
    const float ambientLight = 0.1f;

    float lightIntensity = ambientLight + max(0.f, dot(vsOut.normal, -sunDir));
    lightIntensity = clamp(lightIntensity, 0.f, 1.f);

    const float3 baseColor = texturePool[textureId].Sample(samplerPool[samplerId], vsOut.uv);
    const float3 colorLightPass = baseColor * lightIntensity;

    psOut.color = float4(baseColor, 1.f);
}

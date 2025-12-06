#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;

[[vk::push_constant]]
FCommonPushConstants pc;

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
    FMeshData* mesh = pc.meshData;

    vsOut.position = mul(float4(mesh.positionBuffer[vertexId], 1.f), pc.modelToProj);
    vsOut.normal = mul(mesh.normalBuffer[vertexId], pc.normalModelToWorld);
    vsOut.uv = mesh.uvBuffer[vertexId];
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const float3 sunDir = normalize(float3(-1.f, -1.f, -1.f));
    const float ambientLight = 0.1f;

    float lightIntensity = ambientLight + max(0.f, dot(vsOut.normal, -sunDir));
    lightIntensity = clamp(lightIntensity, 0.f, 1.f);

	const float hue = frac(float(pc.viewData.time / 2.));
    const float3 baseColor = HSVToRGB(float3(hue, 1.f, 1.f));
    const float3 colorLightPass = baseColor * lightIntensity;

    psOut.color = float4(baseColor, 1.f);
}

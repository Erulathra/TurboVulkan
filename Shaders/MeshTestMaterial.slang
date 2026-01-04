#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;
import Modules.Hash;

[[vk::push_constant]]
FCommonPushConstants pc;

struct VSOut
{
    float4 position : SV_Position;
    uint triangleId;
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

    vsOut.triangleId = uint(trunc(vertexId / 3));
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const uint hash = WangHash(vsOut.triangleId);
    const float3 color = HSVToRGB(float3(float(hash) / float(0xFFFFFFFF), 1.f, 1.f));
    psOut.color = float4(color, 1.f);
}

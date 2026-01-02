#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;
import Modules.Hash;

[[vk::push_constant]]
FCommonPushConstants pc;

struct VSOut
{
    float4 position : SV_Position;
    float3 color;
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

    uint seed;
    vsOut.color = RandomColor(int(trunc(float(vertexId) / 3.f)), seed);
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    psOut.color = float4(vsOut.color, 1.f);
}

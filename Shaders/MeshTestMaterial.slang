#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;
import Modules.Hash;

[[vk::push_constant]]
FIndirectPushConstants pc;

struct VSOut
{
    float4 mPosition : SV_Position;
    uint mTriangleId;
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

    vsOut.mTriangleId = uint(trunc(vertexId / 3));
}

[shader("pixel")]
void psMain(in VSOut vsOut, out PSOut psOut)
{
    const uint hash = WangHash(vsOut.mTriangleId);
    const float3 color = HSVToRGB(float3(float(hash) / float(0xFFFFFFFF), 1.f, 1.f));
    psOut.mColor = float4(color, 1.f);
}

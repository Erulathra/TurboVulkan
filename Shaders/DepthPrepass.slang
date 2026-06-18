#include "Modules/Common.slang"

import Modules.BasePassCommon;
import Modules.MeshRendering;
import Modules.Math;
import Modules.ShadingCommon;

[[vk::push_constant]]
FIndirectPushConstants pc;

[shader("vertex")]
void vsMain(
    in uint indexId : SV_VertexID,
	in uint drawId : SV_StartInstanceLocation,
    out float4 position : SV_Position
)
{
	const FIndirectDrawData data = pc.mDrawData[drawId];
    const Ptr<FMeshData> mesh = data.mMeshData;

	const uint vertexId = asuint(mesh.mIndexBuffer[indexId]);
	const float4 modelPosition = float4(mesh.mPositionBuffer[vertexId], 1.f);

    position = mul(modelPosition, data.mModelToProj);
}

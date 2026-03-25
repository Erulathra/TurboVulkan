#include "Modules/Common.slang"

import Modules.MeshRendering;

struct FPushConstants
{
    const Ptr<FViewData> mViewData;
    const Ptr<FIndirectDrawData> mDrawData;
    const Ptr<FBounds> mBounds;

    Ptr<FDrawIndirectCommand> mDrawIndirectCommand;

	uint mNumDraws;
};

[[vk::push_constant()]]
FPushConstants pc;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
	if (threadId.x >= pc.mNumDraws)
	{
	    return;
	}

	FDrawIndirectCommand* draw = &pc.mDrawIndirectCommand[threadId.x];
	draw.mVertexCount = pc.mDrawData[threadId.x].mMeshData.mVertexCount;
	draw.mFirstInstance = threadId.x;
	draw.mInstanceCount = 1;
	draw.mFirstVertex = 0;
}

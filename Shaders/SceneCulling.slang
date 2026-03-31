#include "Modules/Common.slang"

import Modules.MeshRendering;
import Modules.Math;
import Modules.MathTypes;

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

	FIndirectDrawData drawData = pc.mDrawData[threadId.x];

	bool bShouldDraw = true;

	// Transfer bounding sphere to world space
	const uint meshIndex = drawData.mMeshData.mMeshIndex;
	const float3 boundsCenter = (pc.mBounds[meshIndex].mMin + pc.mBounds[meshIndex].mMax) * 0.5f;
	const float boundsRadius = pc.mBounds[meshIndex].mRadius;
	const FSphere boundsSphere = TransformSphere(drawData.mModelToWorld, FSphere(boundsCenter, boundsRadius));

	// Test with frustum planes
	for (uint i = 0; i < 6; ++i)
	{
		const FPlane frustumPlane = pc.mViewData.mViewFrustum.mPlanes[i];
		// const FPlane frustumPlane = pc.mViewData.mViewFrustum.mPlanes[3];
		const float DistanceToPlane = SignedDistanceToPlane(boundsSphere.mCenter, frustumPlane);
        bShouldDraw &= DistanceToPlane > -boundsSphere.mRadius;
	}

	FDrawIndirectCommand draw;
	draw.mVertexCount = drawData.mMeshData.mVertexCount;
	draw.mFirstInstance = threadId.x;
	draw.mInstanceCount = bShouldDraw ? 1 : 0;
	draw.mFirstVertex = 0;

	pc.mDrawIndirectCommand[threadId.x] = draw;
}

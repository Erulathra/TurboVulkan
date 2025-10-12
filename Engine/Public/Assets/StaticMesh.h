#pragma once

namespace Turbo
{
	class FBuffer;

	struct FSubMesh final
	{
	public:
		THandle<FBuffer> mIndicesBuffer;
		THandle<FBuffer> mPositionBuffer;

		THandle<FBuffer> mNormalBuffer;
		THandle<FBuffer> mUVBuffer;
		THandle<FBuffer> mColorBuffer;

		uint32 mVertexCount = 0;
	};
} // Turbo
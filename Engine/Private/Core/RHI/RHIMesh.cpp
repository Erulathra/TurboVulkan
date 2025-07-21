#include "Core/RHI/RHIMesh.h"

namespace Turbo {
	void FRHIMesh::Draw(const vk::CommandBuffer cmd, uint32 instanceCount)
	{
		cmd.bindIndexBuffer(mIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
		cmd.drawIndexed(mIndexBufferSize / sizeof(int32), instanceCount, 0, 0, 0);
	}
} // Turbo
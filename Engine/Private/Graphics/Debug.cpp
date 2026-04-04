#include "Graphics/Debug.h"

#if WITH_DEBUG_RENDERING_FEATURES
#include "Graphics/CommandBuffer.h"

namespace Turbo
{
	FScopedLabelRegion::FScopedLabelRegion(FCommandBuffer& commandBuffer, std::string_view label, glm::float4 color)
		: mCommandBuffer(&commandBuffer)
	{
		TURBO_CHECK(mCommandBuffer)
		commandBuffer.BeginDebugUtilsLabel(label, color);
	}

	FScopedLabelRegion::~FScopedLabelRegion()
	{
		TURBO_CHECK(mCommandBuffer)
		mCommandBuffer->EndDebugUtilsLabel();
	}
} // Turbo

#endif // WITH_DEBUG_RENDERING_FEATURES

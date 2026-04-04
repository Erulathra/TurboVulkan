#pragma once

namespace Turbo
{
#if WITH_DEBUG_RENDERING_FEATURES
	class FCommandBuffer;

	class FScopedLabelRegion final
	{
	public:
		FScopedLabelRegion(FCommandBuffer& commandBuffer, std::string_view label, glm::float4 color = glm::float4(1.f));
		~FScopedLabelRegion();
	private:
		FCommandBuffer* mCommandBuffer;
	};

#define DEBUG_LABEL_REGION(commandBuffer, label) FScopedLabelRegion __label_region__(commandBuffer, label);
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) FScopedLabelRegion __label_region__(commandBuffer, label, color);
#else // WITH_DEBUG_RENDERING_FEATURES
#define DEBUG_LABEL_REGION(commandBuffer, label) {}
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) {}
#endif // else WITH_DEBUG_RENDERING_FEATURES
} // Turbo

#pragma once

#include "Enums.h"
#include "Core/DataStructures/ResourcePool.h"
#include "Core/Math/Color.h"
#include "Core/Math/MathTypes.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class FGPUDevice;

	DECLARE_RESOURCE_HANDLE(CommandBuffer)

	class FCommandBuffer
	{
	public:
		void TransitionImage(FTextureHandle textureHandle, vk::ImageLayout newLayout);
		void ClearImage(FTextureHandle textureHandle, glm::vec4 color = ELinearColor::kBlack);
		void BlitImage(FTextureHandle src, FRect2DInt srcRect, FTextureHandle dst, FRect2DInt dstRect, EFilter filter = EFilter::Linear);

		void BindDescriptorSet(FDescriptorSetHandle descriptorSetHandle, uint32 setIndex);

		void BindPipeline(FPipelineHandle pipelineHandle);
		void Dispatch(const glm::ivec3& groupCount);

		template<typename PushConstantsType>
		void PushConstants(PushConstantsType pushConstants) { PushConstantsImpl(&pushConstants, sizeof(PushConstantsType)); }

		void BeginRendering(FTextureHandle renderTargetHandle);
		void EndRendering();

	public:
		vk::CommandBuffer GetVkCommandBuffer() const { return mVkCommandBuffer; }

	private:
		void Begin();
		void End();

		void Reset();

		vk::CommandBufferSubmitInfo CreateSubmitInfo() const;

		void PushConstantsImpl(void* pushConstants, uint32 size);

	private:
		FGPUDevice* mGpu;
		vk::CommandBuffer mVkCommandBuffer = nullptr;

		std::array<FDescriptorSetHandle, kMaxDescriptorSets> mBoundDescriptorSets;
		FPipelineHandle mCurrentPipeline = {};

		bool mbRecording = false;

	public:
		friend class FGPUDevice;
	};
} // Turbo

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

	struct FRenderingAttachments final
	{
	public:
		FRenderingAttachments& Reset();

		FRenderingAttachments& AddColorAttachment(THandle<FTexture> textureHandle);
		FRenderingAttachments& SetDepthAttachment(THandle<FTexture> textureHandle);

	private:
		uint32 mNumColorAttachments = 0;
		std::array<THandle<FTexture>, 8> mColorAttachments = {};
		THandle<FTexture> mDepthAttachment = {};

	public:
		friend class FCommandBuffer;
	};

	class FCommandBuffer
	{
	public:
		void TransitionImage(THandle<FTexture> textureHandle, vk::ImageLayout newLayout);
		void BufferBarrier(THandle<FBuffer> bufferHandle, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 dstStageMask);

		void ClearImage(THandle<FTexture> textureHandle, glm::vec4 color = ELinearColor::kBlack);
		void BlitImage(THandle<FTexture> src, FRect2DInt srcRect, THandle<FTexture> dst, FRect2DInt dstRect, EFilter filter = EFilter::Linear);

		void CopyBuffer(THandle<FBuffer> src, THandle<FBuffer> dst, vk::DeviceSize size);

		void BindDescriptorSet(THandle<FDescriptorSet> descriptorSetHandle, uint32 setIndex);

		void BindPipeline(THandle<FPipeline> pipelineHandle);
		void Dispatch(const glm::ivec3& groupCount);

		void BeginRendering(const FRenderingAttachments& renderingAttachments);
		void EndRendering();

		void SetViewport(const FViewport& viewport);
		void SetScissor(const FRect2DInt& rect);

		void Draw(uint32 vertexCount, uint32 instanceCount = 1, uint32 firstVertex = 0, uint32 firstInstance = 0);

		template<typename PushConstantsType>
		void PushConstants(PushConstantsType pushConstants) { PushConstantsImpl(&pushConstants, sizeof(PushConstantsType)); }

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

		std::array<THandle<FDescriptorSet>, kMaxDescriptorSets> mBoundDescriptorSets;
		THandle<FPipeline> mCurrentPipeline = {};

		bool mbRecording = false;

	public:
		friend class FGPUDevice;
	};
} // Turbo

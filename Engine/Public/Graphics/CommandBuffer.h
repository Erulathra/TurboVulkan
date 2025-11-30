#pragma once

#include "Enums.h"
#include "Core/Math/Color.h"
#include "Core/Math/MathTypes.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/Resources.h"
#include "Graphics/VulkanHelpers.h"

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

	struct FCopyBufferInfo
	{
		THandle<FBuffer> mSrc = {};
		vk::DeviceSize mSrcOffset = 0;

		THandle<FBuffer> mDst = {};
		vk::DeviceSize mDstOffset = 0;

		vk::DeviceSize mSize = 0;
	};

	class FCommandBuffer
	{
	public:
		void TransitionImage(THandle<FTexture> textureHandle, vk::ImageLayout newLayout);

	public:
		template<TAccessMask SrcAccessMaskType, TStageMask SrcStageMaskType, TAccessMask DstAccessMaskType, TStageMask DstStageMaskType>
		void BufferBarrier(
			THandle<FBuffer> bufferHandle,
			SrcAccessMaskType srcAccessMask,
			SrcStageMaskType srcStageMask,
			DstAccessMaskType dstAccessMask,
			DstStageMaskType dstStageMask,
			vk::DeviceSize offset = 0,
			vk::DeviceSize size = vk::WholeSize
		)
		{
			BufferBarrier_Internal(
				bufferHandle,
				static_cast<vk::AccessFlagBits2>(srcAccessMask),
				static_cast<vk::PipelineStageFlagBits2>(srcStageMask),
				static_cast<vk::AccessFlagBits2>(dstAccessMask),
				static_cast<vk::PipelineStageFlagBits2>(dstStageMask),
				offset,
				size
			);
		}

		void ClearImage(THandle<FTexture> textureHandle, glm::vec4 color = ELinearColor::kBlack);
		void BlitImage(THandle<FTexture> src, FRect2DInt srcRect, THandle<FTexture> dst, FRect2DInt dstRect, EFilter filter = EFilter::Linear);

		void CopyBuffer(THandle<FBuffer> src, THandle<FBuffer> dst, vk::DeviceSize size);
		void CopyBuffer(const FCopyBufferInfo& copyBufferInfo);
		void CopyBufferToTexture(THandle<FBuffer> src, THandle<FTexture> dst, uint32 mipIndex, vk::DeviceSize bufferOffset = 0);

		void BindDescriptorSet(THandle<FDescriptorSet> descriptorSetHandle, uint32 setIndex = 0);
		void BindPipeline(THandle<FPipeline> pipelineHandle);
		void BindIndexBuffer(THandle<FBuffer> indexBuffer);

		void Dispatch(const glm::ivec3& groupCount);

		void BeginRendering(const FRenderingAttachments& renderingAttachments);
		void EndRendering();

		void SetViewport(const FViewport& viewport);
		void SetScissor(const FRect2DInt& rect);

		void Draw(uint32 vertexCount, uint32 instanceCount = 1, uint32 firstVertex = 0, uint32 firstInstance = 0);
		void DrawIndexed(uint32 indexCount, uint32 instanceCount = 1, uint32 firstIndex = 0, int32 vertexOffset = 0, uint32 firstInstance = 0);

		template<TPushConstant PushConstantsType>
		void PushConstants(PushConstantsType pushConstants) { PushConstants_Internal(&pushConstants, sizeof(PushConstantsType)); }

	public:
		vk::CommandBuffer GetVkCommandBuffer() const { return mVkCommandBuffer; }

	private:
		void Begin();
		void End();

		void Reset();

		vk::CommandBufferSubmitInfo CreateSubmitInfo() const;

		void PushConstants_Internal(void* pushConstants, uint32 size);
		void BufferBarrier_Internal(
			THandle<FBuffer> bufferHandle,
			vk::AccessFlags2 srcAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 dstStageMask,
			vk::DeviceSize offset,
			vk::DeviceSize size
		);

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

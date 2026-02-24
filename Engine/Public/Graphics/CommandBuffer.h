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

	enum class EClearColor : uint8
	{
		TransparentBlack, // {0, 0, 0, 0}
		OpaqueBlack, // {0, 0, 0, 1}
		TransparentWhite, // {1, 1, 1, 0}
		OpaqueWhite, // {1, 1, 1, 1}

		Zero = OpaqueBlack,
		One = OpaqueWhite
	};

	enum class ELoadOp : uint8
	{
		Load,
		Clear,
		DontCare
	};

	enum class EStoreOp : uint8
	{
		Store,
		DontCare
	};

	struct FAttachment
	{
		THandle<FTexture> mTexture = {};
		ELoadOp mLoadOp = ELoadOp::Load;
		EStoreOp mStoreOp = EStoreOp::Store;
		EClearColor mClearColor = EClearColor::Zero;
	};

	struct FRenderingAttachments final
	{
		FRenderingAttachments& Reset();

		FRenderingAttachments& AddColorAttachment(const FAttachment& attachment)
		{
			mColorAttachments[mNumColorAttachments++] = attachment;
			return *this;
		}

		FRenderingAttachments& SetDepthAttachment(const FAttachment& attachment)
		{
			mDepthAttachment = attachment;
			return *this;
		}

		uint32 mNumColorAttachments = 0;
		std::array<FAttachment, kMaxColorAttachments> mColorAttachments = {};
		FAttachment mDepthAttachment = {};
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
		void TransitionImage(THandle<FTexture> textureHandle, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void PipelineBarrier(const vk::DependencyInfo& dependencyInfo);

	public:
		void BufferBarrier(
			THandle<FBuffer> bufferHandle,
			vk::AccessFlags2 srcAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 dstStageMask,
			vk::DeviceSize offset = 0,
			vk::DeviceSize size = vk::WholeSize
		);

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

		template<PushConstant PushConstantsType>
		void PushConstants(PushConstantsType pushConstants) { PushConstants_Internal(&pushConstants, sizeof(PushConstantsType)); }

	public:
		vk::CommandBuffer GetVkCommandBuffer() const { return mVkCommandBuffer; }

	private:
		void Begin();
		void End();

		void Reset();

		vk::CommandBufferSubmitInfo CreateSubmitInfo() const;

		void PushConstants_Internal(void* pushConstants, uint32 size);

	private:
		FGPUDevice* mGpu;
		vk::CommandBuffer mVkCommandBuffer = nullptr;

		std::array<THandle<FDescriptorSet>, kMaxDescriptorSets> mBoundDescriptorSets;
		THandle<FPipeline> mCurrentPipeline = {};

		bool mbRecording = false;

	public:
		friend class FGPUDevice;
	};

	constexpr vk::AttachmentLoadOp ToVkLoadOp(ELoadOp loadOp)
	{
		switch (loadOp)
		{
		case ELoadOp::Load:
			return vk::AttachmentLoadOp::eLoad;
		case ELoadOp::Clear:
			return vk::AttachmentLoadOp::eClear;
		case ELoadOp::DontCare:
			return vk::AttachmentLoadOp::eDontCare;
		default:
			TURBO_UNINPLEMENTED()
		}

		return vk::AttachmentLoadOp::eNone;
	}

	constexpr vk::AttachmentStoreOp ToVkStoreOp(EStoreOp storeOp)
	{
		switch (storeOp)
		{
		case EStoreOp::Store:
			return vk::AttachmentStoreOp::eStore;
		case EStoreOp::DontCare:
			return vk::AttachmentStoreOp::eDontCare;
		default:
			TURBO_UNINPLEMENTED();
		}

		return vk::AttachmentStoreOp::eNone;
	}

	constexpr vk::ClearValue ToVkColorClearValue(EClearColor clearColor)
	{
		switch (clearColor)
		{
		case EClearColor::TransparentBlack:
			return vk::ClearValue{{0.f, 0.f, 0.f, 0.f}};
		case EClearColor::OpaqueBlack:
			return vk::ClearValue{{0.f, 0.f, 0.f, 1.f}};
		case EClearColor::TransparentWhite:
			return vk::ClearValue{{1.f, 1.f, 1.f, 0.f}};
		case EClearColor::OpaqueWhite:
			return vk::ClearValue{{1.f, 1.f, 1.f, 1.f}};
		default:
			TURBO_UNINPLEMENTED();
		}

		return {};
	}

	constexpr vk::ClearValue ToVkDepthClearValue(EClearColor clearColor)
	{
		switch (clearColor)
		{
		case EClearColor::Zero:
			return vk::ClearValue{0.f};
		case EClearColor::One:
			return vk::ClearValue{1.f};
		default:
			TURBO_UNINPLEMENTED();
		}

		return {};
	}
} // Turbo

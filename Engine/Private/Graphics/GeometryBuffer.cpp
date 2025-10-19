#include "Graphics/GeometryBuffer.h"

#include "Graphics/Enums.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/CommandBuffer.h"

namespace Turbo
{
	FGeometryBuffer::FGeometryBuffer(FGPUDevice* gpu)
		: mGpu(gpu)
	{
		TURBO_CHECK(gpu)
	}

	void FGeometryBuffer::Init(const glm::ivec2& newResolution)
	{
		const static FName geometryBufferColorName = FName{"GBuffer_Color"};
		const static FName geometryBufferDepthName = FName{"GBuffer_Depth"};

		mResolution = newResolution;

		FTextureBuilder textureBuilder;
		textureBuilder
			.Init(kColorFormat, ETextureType::Texture2D, ETextureFlags::RenderTarget | ETextureFlags::Compute)
			.SetSize(glm::vec3(mResolution, 1))
			.SetNumMips(1)
			.SetName(geometryBufferColorName);

		mColor = mGpu->CreateTexture(textureBuilder);
		TURBO_CHECK(mColor.IsValid())

		textureBuilder
			.Init(kDepthFormat, ETextureType::Texture2D, ETextureFlags::RenderTarget)
			.SetName(geometryBufferDepthName);

		mDepth = mGpu->CreateTexture(textureBuilder);
		TURBO_CHECK(mDepth.IsValid())
	}

	void FGeometryBuffer::Destroy()
	{
		mGpu->DestroyTexture(mColor);
		mGpu->DestroyTexture(mDepth);
	}

	void FGeometryBuffer::Resize(const glm::ivec2& newResolution)
	{
		Destroy();
		Init(newResolution);
	}

	void FGeometryBuffer::BlitResultToTexture(FCommandBuffer* cmd, THandle<FTexture> presentTextureHandle)
	{
		FTexture* presentTexture = mGpu->AccessTexture(presentTextureHandle);
		TURBO_CHECK(presentTexture)

		const FRect2DInt srcRect = FRect2DInt::FromSize(mResolution);
		const FRect2DInt dstRect = FRect2DInt::FromSize(presentTexture->GetSize());
		cmd->TransitionImage(mColor, vk::ImageLayout::eTransferSrcOptimal);
		cmd->TransitionImage(presentTextureHandle, vk::ImageLayout::eTransferDstOptimal);
		cmd->BlitImage(mColor, srcRect, presentTextureHandle, dstRect);
		cmd->TransitionImage(presentTextureHandle, vk::ImageLayout::eGeneral);
	}
} // Turbo
#pragma once

#include "Graphics/Resources.h"

namespace Turbo
{
	class FCommandBuffer;

	class FGeometryBuffer
	{
		GENERATED_BODY(FGeometryBuffer)
	public:
		explicit FGeometryBuffer(FGPUDevice* gpu);

	public:
		void Init(const glm::ivec2& newResolution);
		void Destroy();

		void Resize(const glm::ivec2& newResolution);

		void BlitResultToTexture(FCommandBuffer* cmd, FTextureHandle swapChainTexture);

	public:
		[[nodiscard]] FTextureHandle GetColor() const { return mColor; }
		[[nodiscard]] FTextureHandle GetDepth() const { return mDepth; }
		[[nodiscard]] const glm::ivec2& GetResolution() const { return mResolution; }

	private:
		FGPUDevice* mGpu = nullptr;

		FTextureHandle mColor = {};
		FTextureHandle mDepth = {};

		glm::ivec2 mResolution = {};
	};
} // Turbo
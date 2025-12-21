#pragma once

#include "Graphics/Resources.h"

namespace Turbo
{
	class FCommandBuffer;

	class FGeometryBuffer
	{
		GENERATED_BODY(FGeometryBuffer)

	public:
		static constexpr vk::Format kColorFormat = vk::Format::eR16G16B16A16Sfloat;
		static constexpr vk::Format kDepthFormat = vk::Format::eD32Sfloat;

	public:
		explicit FGeometryBuffer(FGPUDevice* gpu);
		DELETE_COPY(FGeometryBuffer);

	public:
		void Init(const glm::ivec2& newResolution);
		void Destroy();

		void Resize(const glm::ivec2& newResolution);

		void BlitResultToTexture(FCommandBuffer& cmd, THandle<FTexture> swapChainTexture);

	public:
		[[nodiscard]] THandle<FTexture> GetColor() const { return mColor; }
		[[nodiscard]] THandle<FTexture> GetDepth() const { return mDepth; }
		[[nodiscard]] const glm::ivec2& GetResolution() const { return mResolution; }

	private:
		FGPUDevice* mGpu = nullptr;

		THandle<FTexture> mColor = {};
		THandle<FTexture> mDepth = {};

		glm::ivec2 mResolution = {};
	};
} // Turbo
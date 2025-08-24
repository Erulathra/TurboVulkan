#pragma once

#include "Enums.h"
#include "Core/DataStructures/ResourcePool.h"
#include "Core/Math/Color.h"
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

	private:
		void Begin();
		void End();

		void Reset();

		vk::CommandBufferSubmitInfo CreateSubmitInfo() const;

	private:
		FGPUDevice* mDevice;
		vk::CommandBuffer mVkCommandBuffer = nullptr;

		std::array<FDescriptorSetHandle, kMaxDescriptorSets> mBoundDescriptorSets;
		FPipelineHandle mCurrentPipeline = {};

		bool mbRecording = false;

	public:
		friend class FGPUDevice;
	};
} // Turbo

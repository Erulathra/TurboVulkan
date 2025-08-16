#pragma once

#include "Graphics/GraphicsCore.h"

#define DECLARE_RESOURCE_HANDLE(RESOURCE_NAME)						\
	struct F##RESOURCE_NAME##Handle final							\
	{																\
		FResourceHandle Index = kInvalidHandle;						\
		bool IsValid() const { return Index != kInvalidHandle; }	\
		void Reset() { Index = kInvalidHandle; }					\
	}

#define RESOURCE_BODY()			\
	public:						\
		friend class GPUDevice;	\
	private:

namespace Turbo
{
	using FResourceHandle = uint32;
	inline constexpr FResourceHandle kInvalidHandle = std::numeric_limits<uint32>::max();

	/** Handles */

	DECLARE_RESOURCE_HANDLE(Buffer);
	DECLARE_RESOURCE_HANDLE(Texture);
	DECLARE_RESOURCE_HANDLE(ShaderState);
	DECLARE_RESOURCE_HANDLE(Sampler);
	DECLARE_RESOURCE_HANDLE(DescriptorSetLayout);
	DECLARE_RESOURCE_HANDLE(DescriptorSet);
	DECLARE_RESOURCE_HANDLE(Pipeline);

	/** Handles end */

	/** Vulkan object abstractions */

	class FBuffer final
	{
		RESOURCE_BODY()

	private:
		vk::Buffer mBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;

		vk::BufferUsageFlags2 mUsageFlags = {};
		vk::DeviceSize mDeviceSize = {};

		FBufferHandle mHandle = {};

		FName mName;
	};

	class FSampler final
	{
		RESOURCE_BODY()

	private:
		vk::Sampler mSampler = nullptr;

		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		FName mName;
	};

	class FTexture final
	{
		RESOURCE_BODY()

	private:
		vk::Image mImage = nullptr;
		vk::ImageView mImageView = nullptr;
		vk::Format mFormat = vk::Format::eUndefined;
		vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;
		vma::Allocation mImageAllocation = nullptr;

		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mMips = 1;

		FTextureHandle mHandle = {};

		FName mName;
	};

	class FShaderState final
	{
		RESOURCE_BODY()

	private:
		std::array<vk::PipelineShaderStageCreateInfo, kMaxShaderStages> mShaderStageCrateInfo;

		FName mName;

		uint32 mNumActiveShaders = 0;
		bool mbGraphicsPipeline = false;
	};

	class FDescriptorSetLayout final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSetLayout mLayout = nullptr;

		uint16 mSetIndex = 0;

		FDescriptorSetLayoutHandle mHandle = {};
	};

	class FDescriptorSet final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSet mDescriptorSet = nullptr;
	};

	class FPipeline final
	{
		RESOURCE_BODY()

	private:
		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mLayout = nullptr;

		vk::PipelineBindPoint mBindPoint = {};

		FShaderStateHandle mShaderState = {};

		std::array<FDescriptorSetLayoutHandle, kMaxDescriptorSetLayouts> mDescriptorLayouts;
		uint32 mNumActiveLayouts = 0;

		FPipelineHandle mHandle = {};
		bool mbGraphicsPipeline = true;
	};

	/** Vulkan object abstractions end */

} // Turbo

#undef RESOURCE_BODY
#undef DECLARE_RESOURCE_HANDLE

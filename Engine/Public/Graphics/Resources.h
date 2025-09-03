#pragma once

#include "DestoryQueue.h"
#include "Core/DataStructures/ArrayHeap.h"
#include "Core/DataStructures/ResourcePool.h"
#include "Graphics/GraphicsCore.h"

#define RESOURCE_BODY()					\
	public:								\
		friend class FGPUDevice;		\
		friend class FCommandBuffer;	\
	private:

#define DESTROYER_BODY()				\
	public:								\
		friend class FGPUDevice;

namespace Turbo
{

	/** Handles */

	DECLARE_RESOURCE_HANDLE(Buffer)
	DECLARE_RESOURCE_HANDLE(Texture)
	DECLARE_RESOURCE_HANDLE(ShaderState)
	DECLARE_RESOURCE_HANDLE(Sampler)
	DECLARE_RESOURCE_HANDLE(DescriptorSetLayout)
	DECLARE_RESOURCE_HANDLE(DescriptorPool)
	DECLARE_RESOURCE_HANDLE(DescriptorSet)
	DECLARE_RESOURCE_HANDLE(Pipeline)

	/** Handles end */

	/** Vulkan object abstractions */

	class FBuffer final
	{
		RESOURCE_BODY()

	private:
		vk::Buffer mVkBuffer = nullptr;
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
		vk::Sampler mVkSampler = nullptr;

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
	public:
		glm::ivec3 GetSize() const { return glm::ivec3{mWidth, mHeight, mDepth}; }

	private:
		vk::Image mImage = nullptr;
		vk::ImageView mImageView = nullptr;
		vk::Format mFormat = vk::Format::eUndefined;
		vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;
		vma::Allocation mImageAllocation = nullptr;

		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;

		FTextureHandle mHandle = {};

		FName mName;
	};

	class FTextureDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Image mImage = nullptr;
		vk::ImageView mImageView = nullptr;
		vma::Allocation mImageAllocation = nullptr;
		FTextureHandle mHandle = {};
	};

	class FShaderState final
	{
		RESOURCE_BODY()

	private:
		std::array<vk::PipelineShaderStageCreateInfo, kMaxShaderStages> mShaderStageCrateInfo;

		FName mName;

		uint32 mNumActiveShaders = 0;
		bool mbGraphicsPipeline = true;
	};

	class FShaderStateDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		std::array<vk::ShaderModule, kMaxShaderStages> mModules;
		uint32 mNumActiveShaders = 0;

		FShaderStateHandle mHandle;
	};

	struct FBinding
	{
		vk::DescriptorType mType = {};
		uint16 mIndex = 0;
		uint16 mCount = 0;

		FName mName;
	};

	class FDescriptorSetLayout final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSetLayout mLayout = nullptr;
		TArrayHeap<vk::DescriptorSetLayoutBinding, kMaxDescriptorsPerSet> mVkBindings;
		TArrayHeap<FBinding, kMaxDescriptorsPerSet> mBindings;
		uint16 mNumBindings = 0;
		uint16 mSetIndex = 0;

		FDescriptorSetLayoutHandle mHandle = {};
	};

	class FDescriptorSetLayoutDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorSetLayout mLayout = nullptr;
		FDescriptorSetLayoutHandle mHandle = {};
	};

	class FDescriptorSet final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSet mVkDescriptorSet = nullptr;

		FDescriptorPoolHandle mOwnerPool = {};
	};

	class FDescriptorPool final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorPool mDescriptorPool;
		std::vector<FDescriptorSetHandle> mDescriptorSets;

		FName mName;
	};

	class FDescriptorPoolDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorPool mDescriptorPool;
		FDescriptorPoolHandle mhandle;
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

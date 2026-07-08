#pragma once

#include "DestoryQueue.h"
#include "Enums.h"
#include "Core/DataStructures/ArrayHeap.h"
#include "Graphics/GraphicsCore.h"

#define DESTROYER_BODY()				\
	public:								\
		friend class FGPUDevice;

namespace Turbo
{
	struct FPipelineBuilder;

	enum class EResourceType : uint8
	{
		Texture,
		RWTexture,
		Sampler,
		TLAS,

		None,
	};

	namespace BindlessResourcesBindings
	{
		constexpr uint32 kSampledImage = 0;
		constexpr uint32 kStorageImage = 1;
		constexpr uint32 kSampler = 2;
	}

	struct FBindlessResourceUpdateRequest
	{
		EResourceType mType = EResourceType::None;
		uint32 mBindingIndex = std::numeric_limits<uint32>::max();
		FHandle mHandle = FHandle();
	};

	/** Vulkan object abstractions */

	struct FBuffer
	{
		[[nodiscard]] bool IsValid() const { return  mVkBuffer != nullptr; }

		vk::Buffer mVkBuffer = nullptr;

		FDeviceSize mDeviceSize = {};

		FDeviceAddress mDeviceAddress = {};
		byte* mMappedAddress = nullptr;
	};

	struct FBufferCold
	{
		vma::Allocation mAllocation = nullptr;
		EBufferFlags mBufferFlags = EBufferFlags::None;

		THandle<FBuffer> mHandle = {};

		FName mName;
	};

	class FBufferDestroyer : IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Buffer mVkBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;
		THandle<FBuffer> mHandle;
	};

	struct FSampler
	{
		vk::Sampler mVkSampler = nullptr;
	};

	struct FSamplerCold
	{
		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		FName mName = {};
	};

	class FSamplerDestroyer : IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Sampler mVkSampler;
		THandle<FSampler> mHandle;
	};

	struct FTexture
	{
		vk::Image mVkImage = nullptr;
		vk::ImageView mVkImageView = nullptr;
		vma::Allocation mImageAllocation = nullptr;
		uint32 mBindIndex = std::numeric_limits<uint32>::max();

		ETextureFlags mFlags = ETextureFlags::Invalid;
	};

	struct FTextureCold
	{
		[[nodiscard]] glm::int2 GetSize2D() const { return glm::ivec2{mWidth, mHeight}; }
		[[nodiscard]] glm::int3 GetSize() const { return glm::ivec3{mWidth, mHeight, mDepth}; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }

		vk::Format mFormat = vk::Format::eUndefined;

		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;

		THandle<FTexture> mHandle = {};
		FName mName = {};
	};

	class FTextureDestroyer : public IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Image mImage = nullptr;
		vk::ImageView mImageView = nullptr;
		vma::Allocation mImageAllocation = nullptr;
		THandle<FTexture> mHandle = {};
	};

	struct FShaderState
	{
		std::array<vk::PipelineShaderStageCreateInfo, kMaxShaderStages> mShaderStageCrateInfo;

		FName mName;

		uint32 mNumActiveShaders = 0;
		bool mbGraphicsPipeline = true;
	};

	class FShaderStateDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		std::array<vk::ShaderModule, kMaxShaderStages> mModules;
		uint32 mNumActiveShaders = 0;

		THandle<FShaderState> mHandle;
	};

	struct FBinding
	{
		vk::DescriptorType mType = {};
		uint16 mIndex = 0;
		uint16 mCount = 0;
		vk::DescriptorBindingFlags mFlags = {};

		FName mName;
	};

	struct FDescriptorSetLayout
	{
		vk::DescriptorSetLayout mVkLayout = nullptr;
		TArrayHeap<vk::DescriptorSetLayoutBinding, kMaxDescriptorsPerSet> mVkBindings;
		TArrayHeap<FBinding, kMaxDescriptorsPerSet> mBindings;
		uint16 mNumBindings = 0;
		uint16 mSetIndex = 0;

		THandle<FDescriptorSetLayout> mHandle = {};
	};

	struct FDescriptorPool;

	struct FDescriptorSet
	{
		vk::DescriptorSet mVkDescriptorSet = nullptr;

		THandle<FDescriptorPool> mOwnerPool = {};
	};

	struct FDescriptorPool
	{
		[[nodiscard]] vk::DescriptorPool GetDescriptorPool() const { return mVkDescriptorPool; }

		vk::DescriptorPool mVkDescriptorPool;
		std::vector<THandle<FDescriptorSet>> mDescriptorSets;

		FName mName;
	};

	class FDescriptorSetLayoutDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorSetLayout mVkLayout = nullptr;
		THandle<FDescriptorSetLayout> mHandle = {};
	};

	class FDescriptorPoolDestroyer : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorPool mVkDescriptorPool;
		THandle<FDescriptorPool> mhandle;
	};

	struct FPipeline
	{
		vk::Pipeline mVkPipeline = nullptr;
		vk::PipelineLayout mVkLayout = nullptr;

		vk::PipelineBindPoint mVkBindPoint = {};

		bool mbGraphicsPipeline = true;
	};

	struct FPipelineCold
	{
		THandle<FShaderState> mShaderState = {};
		FPipelineBuilder* mPipelineBuilder = nullptr; // Allows to recompile pipeline at runtime
	};

	class FPipelineDestroyer : IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mLayout = nullptr;
		THandle<FPipeline> mHandle = {};
	};

	struct FAccelerationStructure
	{
		vk::AccelerationStructureKHR mVkAccelerationStructure;
		FDeviceAddress mDeviceAddress;
		THandle<FBuffer> mBuffer;

		FName mName = {};
	};

	class FAccelerationStructureDestroyer : IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	protected:
		vk::AccelerationStructureKHR mAccelerationStructure;
		THandle<FBuffer> mBuffer;
		THandle<FAccelerationStructure> mHandle;
	};

	/** Vulkan object abstractions end */

} // Turbo

#undef RESOURCE_BODY

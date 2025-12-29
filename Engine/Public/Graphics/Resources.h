#pragma once

#include "DestoryQueue.h"
#include "Core/DataStructures/ArrayHeap.h"
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
	enum class EResourceType : uint8
	{
		Texture,
		RWTexture,
		Sampler,

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
		FHandle mHandle = FHandle();

		FBindlessResourceUpdateRequest(EResourceType resource, const FHandle& resourceHandle)
			: mType(resource)
			, mHandle(resourceHandle)
		{
		}
	};

	/** Vulkan object abstractions */

	class FBuffer final
	{
		RESOURCE_BODY()

	public:
		[[nodiscard]] void* GetMappedAddress() const { return mMappedAddress; }
		[[nodiscard]] FDeviceAddress GetDeviceAddress() const { return mDeviceAddress; }
		[[nodiscard]] FName GetName() const { return mName; }
		[[nodiscard]] vk::DeviceSize GetSize() const { return mDeviceSize; }

	private:
		vk::Buffer mVkBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;

		vk::BufferUsageFlags mUsageFlags = {};
		vk::DeviceSize mDeviceSize = {};

		FDeviceAddress mDeviceAddress = {};
		void* mMappedAddress = nullptr;

		THandle<FBuffer> mHandle = {};

		FName mName;
	};

	class FBufferDestroyer final : IDestroyer
	{
		RESOURCE_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Buffer mVkBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;
		THandle<FBuffer> mHandle;
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

	class FSamplerDestroyer final : IDestroyer
	{
		DESTROYER_BODY()
	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Sampler mVkSampler;
		THandle<FSampler> mHandle;
	};

	class FTexture final
	{
		RESOURCE_BODY()
	public:
		[[nodiscard]] glm::int2 GetSize2D() const { return glm::ivec2{mWidth, mHeight}; }
		[[nodiscard]] glm::int2 GetSize() const { return glm::ivec3{mWidth, mHeight, mDepth}; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }

		vk::Image mVkImage = nullptr;
		vk::ImageView mVkImageView = nullptr;
		vk::Format mFormat = vk::Format::eUndefined;
		vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;
		vma::Allocation mImageAllocation = nullptr;

		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;

		THandle<FTexture> mHandle = {};

		FName mName = {};
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
		THandle<FTexture> mHandle = {};
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

	class FDescriptorSetLayout final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSetLayout mVkLayout = nullptr;
		TArrayHeap<vk::DescriptorSetLayoutBinding, kMaxDescriptorsPerSet> mVkBindings;
		TArrayHeap<FBinding, kMaxDescriptorsPerSet> mBindings;
		uint16 mNumBindings = 0;
		uint16 mSetIndex = 0;

		THandle<FDescriptorSetLayout> mHandle = {};
	};

	class FDescriptorPool;

	class FDescriptorSet final
	{
		RESOURCE_BODY()

	private:
		vk::DescriptorSet mVkDescriptorSet = nullptr;

		THandle<FDescriptorPool> mOwnerPool = {};
	};

	class FDescriptorPool final
	{
		RESOURCE_BODY()

	public:
		[[nodiscard]] vk::DescriptorPool GetDescriptorPool() const { return mVkDescriptorPool; }

	private:
		vk::DescriptorPool mVkDescriptorPool;
		std::vector<THandle<FDescriptorSet>> mDescriptorSets;

		FName mName;
	};

	class FDescriptorSetLayoutDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorSetLayout mVkLayout = nullptr;
		THandle<FDescriptorSetLayout> mHandle = {};
	};

	class FDescriptorPoolDestroyer final : public IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::DescriptorPool mVkDescriptorPool;
		THandle<FDescriptorPool> mhandle;
	};

	class FPipeline final
	{
		RESOURCE_BODY()

	private:
		vk::Pipeline mVkPipeline = nullptr;
		vk::PipelineLayout mVkLayout = nullptr;

		vk::PipelineBindPoint mVkBindPoint = {};

		THandle<FShaderState> mShaderState = {};

		std::array<THandle<FDescriptorSetLayout>, kMaxDescriptorSetLayouts> mDescriptorLayoutsHandles;
		uint32 mNumActiveLayouts = 0;

#if 0
		FDepthStencilBuilder mDepthStencil;
		FBlendStateBuilder mBlendState;
		FRasterizationBuilder mRasterization;
#endif

		THandle<FPipeline> mHandle = {};
		bool mbGraphicsPipeline = true;
	};

	class FPipelineDestroyer final : IDestroyer
	{
		DESTROYER_BODY()

	public:
		virtual void Destroy(FGPUDevice& GPUDevice) override;

	private:
		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mLayout = nullptr;
		THandle<FPipeline> mHandle = {};
	};

	/** Vulkan object abstractions end */

} // Turbo

#undef RESOURCE_BODY

#pragma once

#include "Resources.h"
#include "Core/Name.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/Enums.h"

#define BUILDER_BODY()			\
	public:						\
		friend class FGPUDevice;	\
	private:

namespace Turbo
{
	class FWindow;

	class FGPUDeviceBuilder final
	{
		BUILDER_BODY()

	public:
		FGPUDeviceBuilder& SetWindow(const std::shared_ptr<FWindow>& window) { mWindow = window; return *this; }

	private:
		std::shared_ptr<FWindow> mWindow = nullptr;
	};

	class FBufferBuilder final
	{
		BUILDER_BODY()

	public:
		FBufferBuilder& Reset() { mSize = 0; mInitialData = nullptr; return *this; }
		FBufferBuilder& Init(vk::BufferUsageFlags2 flags, EResourceUsageType usage, uint32 size)
			{ mUsageFlags = flags; mUsageType = usage; mSize = size; return *this; }
		FBufferBuilder& SetData(void* data) { mInitialData = data; return *this; }

		FBufferBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		vk::BufferUsageFlags2 mUsageFlags = {};
		EResourceUsageType mUsageType = EResourceUsageType::Immutable;
		uint32 mSize = 0;
		void* mInitialData = nullptr;

		FName mName;
	};

	class FTextureBuilder final
	{
		BUILDER_BODY()

	public:
		FTextureBuilder& Init(vk::Format format, ETextureType type, ETextureFlags flags)
			{ mFormat = format; mType = type; mFlags = flags;  return *this; }
		FTextureBuilder& SetSize(glm::ivec3 size) { mWidth = size.x; mHeight = size.y; mDepth = size.z; return *this; }
		FTextureBuilder& SetNumMips(uint8 numMips) { mNumMips = numMips; return *this; }

		FTextureBuilder& SetData(void* data) { mInitialData = data; return *this; }

		FTextureBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		void* mInitialData = nullptr;
		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;
		ETextureFlags mFlags = ETextureFlags::Invalid;

		vk::Format mFormat = vk::Format::eUndefined;
		ETextureType mType = ETextureType::Texture2D;

		FName mName;
	};

	class FSamplerBuilder final
	{
		BUILDER_BODY()

	public:
		FSamplerBuilder& SetMinMagFilter(vk::Filter min, vk::Filter mag) { mMinFilter = min; mMagFilter = mag; return *this; }
		FSamplerBuilder& SetMipFilter(vk::SamplerMipmapMode filter) { mMipFilter = filter; return *this; }

		FSamplerBuilder& SetAddressU(vk::SamplerAddressMode u) { mAddressModeU = u; return *this; }
		FSamplerBuilder& SetAddressUV(vk::SamplerAddressMode u, vk::SamplerAddressMode v) { mAddressModeU = u; mAddressModeV = v; return *this; }
		FSamplerBuilder& SetAddressUVW(vk::SamplerAddressMode u, vk::SamplerAddressMode v, vk::SamplerAddressMode w)
			{ mAddressModeU = u; mAddressModeV = v; mAddressModeV = w; return *this; }

		FSamplerBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		vk::Filter mMinFilter = vk::Filter::eNearest;
		vk::Filter mMagFilter = vk::Filter::eNearest;
		vk::SamplerMipmapMode mMipFilter = vk::SamplerMipmapMode::eNearest;

		vk::SamplerAddressMode mAddressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode mAddressModeW = vk::SamplerAddressMode::eRepeat;

		FName mName;
	};

	class FDescriptorSetLayoutBuilder final
	{
		BUILDER_BODY()

	public:
		FDescriptorSetLayoutBuilder& Reset() { mNumBindings = 0; mSetIndex = 0; return *this; }
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, uint16 start, uint16 count, FName name = FName())
		{
			mBindings[mNumBindings] = {type, start, count, name};
			++mNumBindings;
			return *this;
		}
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, uint16 id, FName name = FName())
		{
			return AddBinding(type, id, 1, name);
		}
		FDescriptorSetLayoutBuilder& SetSetIndex(uint16 index) { mSetIndex = index; return *this; }

		FDescriptorSetLayoutBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		std::array<FBinding, kMaxDescriptorsPerSet> mBindings;
		uint16 mNumBindings = 0;
		uint16 mSetIndex = 0;

		FName mName;
	};

	class FDescriptorSetBuilder final
	{
		BUILDER_BODY()

	public:
		FDescriptorSetBuilder& Reset() { mNumResources = 0; return *this; }
		FDescriptorSetBuilder& SetLayout(FDescriptorSetLayoutHandle layout) { mLayout = layout; return *this; }

		FDescriptorSetBuilder& SetTexture(FTextureHandle texture, uint16 binding)
		{
			mSamplers[mNumResources] = FSamplerHandle{};
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = texture.Index;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetBuffer(FBufferHandle buffer, uint16 binding)
		{
			mSamplers[mNumResources] = FSamplerHandle{};
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = buffer.Index;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetSampler(FSamplerHandle sampler, uint16 binding)
		{
			mSamplers[mNumResources] = sampler;
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = kInvalidHandle;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		std::array<FResourceHandle, kMaxDescriptorsPerSet> mResources;
		std::array<FSamplerHandle, kMaxDescriptorsPerSet> mSamplers;
		std::array<uint16, kMaxDescriptorsPerSet> mBindings;

		FDescriptorSetLayoutHandle mLayout = {};
		uint32 mNumResources = 0;

		FName mName;
	};

	class FRasterizationBuilder final
	{
		BUILDER_BODY()

	public:
		FRasterizationBuilder& SetCullMode(vk::CullModeFlags cullMode) { mCullMode = cullMode; return *this; }
		FRasterizationBuilder& SetPolygonMode(vk::CullModeFlags polygonMode) { mCullMode = polygonMode; return *this; }

	private:
		vk::CullModeFlags mCullMode = vk::CullModeFlagBits::eBack;
		vk::PolygonMode mPolygonMode = vk::PolygonMode::eFill;
	};

	class FDepthStencilBuilder final
	{
		BUILDER_BODY()

	public:
		FDepthStencilBuilder& SetDepth(bool bWrite, vk::CompareOp compareOperator)
		{
			mbEnableDepth = true;
			mbEnableWriteDepth = bWrite;
			mDepthCompareOperator = compareOperator;

			return *this;
		}

	private:
		vk::CompareOp mDepthCompareOperator = vk::CompareOp::eGreater;

		bool mbEnableDepth : 1 = false;
		bool mbEnableWriteDepth : 1 = false;
		bool mbEnableStencil : 1 = false;
		uint8 mPadding : 5 = 0;
	};

	class FBlendState final
	{
		BUILDER_BODY()

	public:
		FBlendState& Init(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			InitColor(source, destination, blendOperator);
			InitAlpha(source, destination, blendOperator);

			return *this;
		}

		FBlendState& InitColor(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			mbBlendEnabled = true;

			mSourceColorBlendFactor = source;
			mDestinationColorBlendFactor = destination;
			mColorBlendOperator = blendOperator;

			return *this;
		}

		FBlendState& InitAlpha(vk::BlendFactor source, vk::BlendFactor destination, vk::BlendOp blendOperator)
		{
			mbBlendEnabled = true;

			mSourceAlphaBlendFactor = source;
			mDestinationAlphaBlendFactor = destination;
			mAlphaBlendOperator = blendOperator;

			return *this;
		}

		FBlendState& SetWriteMask(vk::ColorComponentFlags writeMask) { mColorComponentMask = writeMask; return *this; }

		FBlendState& InitAlphaBlending()
		{
			InitColor(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd);
			InitColor(vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd);

			return *this;
		}

		FBlendState& InitAdditiveBlending()
		{
			InitColor(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, vk::BlendOp::eAdd);
			InitColor(vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd);

			return *this;
		}

	private:
		vk::BlendFactor mSourceColorBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor mDestinationColorBlendFactor = vk::BlendFactor::eOne;
		vk::BlendOp mColorBlendOperator = vk::BlendOp::eAdd;

		vk::BlendFactor mSourceAlphaBlendFactor = vk::BlendFactor::eOne;
		vk::BlendFactor mDestinationAlphaBlendFactor = vk::BlendFactor::eOne;
		vk::BlendOp mAlphaBlendOperator = vk::BlendOp::eAdd;

		vk::ColorComponentFlags mColorComponentMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;

		bool mbBlendEnabled = false;
	};

	class FBlendStateBuilder final
	{
		BUILDER_BODY()

	public:
		FBlendStateBuilder& Reset() { mActiveStates = 0; return *this; }
		FBlendState& AddBlendState()
		{
			mBlendStates[mActiveStates] = FBlendState();
			++mActiveStates;

			return mBlendStates[mActiveStates - 1];
		}

	private:
		std::array<FBlendState, kMaxImageOutputs> mBlendStates;
		uint32 mActiveStates = 0;
	};

	struct FShaderStage final
	{
		std::string mShaderName;
		vk::ShaderStageFlagBits mStage;
		std::string mEntryPoint;
	};

	constexpr cstring GetShaderEntryPointName(vk::ShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return "vsMain";
		case vk::ShaderStageFlagBits::eFragment:
			return "psMain";
		default:
			return "main";
		}
	}

	class FShaderStateBuilder final
	{
		BUILDER_BODY()
	public:
		FShaderStateBuilder& Reset() { mStagesCount = 0; return *this; }
		FShaderStateBuilder& AddStage(const std::string_view shaderName, vk::ShaderStageFlagBits stage, std::string_view entryPoint = "")
		{
			if (entryPoint.empty())
			{
				entryPoint = GetShaderEntryPointName(stage);
			}

			mStages[mStagesCount] = {std::string(shaderName), stage, std::string(entryPoint)};
			++mStagesCount;
			return *this;
		}
		FShaderStateBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		std::array<FShaderStage, kMaxShaderStages> mStages;
		uint32 mStagesCount = 0;

		FName mName;
	};

	class FPipelineBuilder final
	{
		BUILDER_BODY()

	public:
		FRasterizationBuilder& GetRasterization() { return mRasterization; }
		FDepthStencilBuilder& GetDepthStencil() { return mDepthStencil; }
		FBlendStateBuilder& GetBlendState() { return mBlendState; }
		FShaderStateBuilder& GetShaderState() { return mShaderState; }

		FPipelineBuilder& AddDescriptorSetLayout(FDescriptorSetLayoutHandle handle)
			{ mDescriptorSetLayouts[mNumActiveLayouts++] = handle; return *this; }

	private:
		FRasterizationBuilder mRasterization;
		FDepthStencilBuilder mDepthStencil;
		FBlendStateBuilder mBlendState;
		FShaderStateBuilder mShaderState;

		std::array<FDescriptorSetLayoutHandle, kMaxDescriptorSetLayouts> mDescriptorSetLayouts;
		uint32 mNumActiveLayouts = 0;

		FName mName = FName();
	};
}

#undef BUILDER_BODY
#pragma once

#include "Resources.h"
#include "Core/Name.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/Enums.h"
#include "Graphics/VulkanHelpers.h"

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
	};

	class FBufferBuilder final
	{
		BUILDER_BODY()

	public:
		static FBufferBuilder CreateStagingBuffer(const void* data, uint32 size);
		static FBufferBuilder CreateStagingBuffer(uint32 size);
		static FBufferBuilder CreateStagingBuffer(std::span<byte> data);

	public:
		FBufferBuilder& Reset() { mSize = 0; mInitialData = nullptr; return *this; }
		FBufferBuilder& Init(vk::BufferUsageFlags usageFlags, EBufferFlags bufferFlags, size_t size)
			{ mUsageFlags = usageFlags; mBufferFlags = bufferFlags; mSize = size; return *this; }
		FBufferBuilder& SetData(const void* data) { mInitialData = data; return *this; }

		FBufferBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		vk::BufferUsageFlags mUsageFlags = {};
		EBufferFlags mBufferFlags = EBufferFlags::None;
		size_t mSize = 0;

		const void* mInitialData = nullptr;

		FName mName;
	};

	enum class EDummyTextureType
	{
		Black,
		White,
		Normal
	};

	class FTextureBuilder final
	{
		BUILDER_BODY()

	public:
		FTextureBuilder& Init(vk::Format format, ETextureType type, ETextureFlags flags = ETextureFlags::Default)
			{ mFormat = format; mType = type; mFlags = flags;  return *this; }
		FTextureBuilder& SetSize(glm::uint3 size) { mWidth = size.x; mHeight = size.y; mDepth = size.z; return *this; }
		FTextureBuilder& SetNumMips(uint8 numMips) { mNumMips = numMips; return *this; }
		FTextureBuilder& SetBindTexture(bool bBindTexture) { mbBindTexture = bBindTexture; return *this; }

		FTextureBuilder& SetName(FName name) { mName = name; return *this; }

		uint16 mWidth = 1;
		uint16 mHeight = 1;
		uint16 mDepth = 1;
		uint8 mNumMips = 1;
		ETextureFlags mFlags = ETextureFlags::Invalid;

		vk::Format mFormat = vk::Format::eUndefined;
		ETextureType mType = ETextureType::Texture2D;

		bool mbBindTexture = true;

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
		FSamplerBuilder& SetAddress(vk::SamplerAddressMode mode)
			{ mAddressModeU = mode; mAddressModeV = mode; mAddressModeV = mode; return *this; }

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

	class FDescriptorPoolBuilder final
	{
		BUILDER_BODY()
	public:
		FDescriptorPoolBuilder();
		FDescriptorPoolBuilder& Reset();
		FDescriptorPoolBuilder& SetPoolRatio(vk::DescriptorType type, float ratio);

		FDescriptorPoolBuilder& SetFlags(vk::DescriptorPoolCreateFlags flags) {mFlags = flags; return *this;}
		FDescriptorPoolBuilder& SetMaxSets(uint32 maxSets) { mMaxSets = maxSets; return *this; }
		FDescriptorPoolBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		std::unordered_map<vk::DescriptorType, float /** Ratio **/> mPoolSizes = {};
		uint32 mMaxSets = 0;

		vk::DescriptorPoolCreateFlags mFlags;

		FName mName;
	};

	class FDescriptorSetLayoutBuilder final
	{
		BUILDER_BODY()

	public:
		FDescriptorSetLayoutBuilder& Reset() { *this = {}; return *this; }
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, uint16 start, uint16 count, vk::DescriptorBindingFlags flags = {}, FName name = FName())
		{
			TURBO_CHECK_MSG(count > 0, "Binding count must be greater than 0.")

			mBindings[mNumBindings] = {type, start, count, flags, name};
			++mNumBindings;

			return *this;
		}
		FDescriptorSetLayoutBuilder& AddBinding(vk::DescriptorType type, uint16 id, vk::DescriptorBindingFlags flags = {}, FName name = FName())
		{
			return AddBinding(type, id, 1, flags, name);
		}
		FDescriptorSetLayoutBuilder& SetIndex(uint16 index) { mSetIndex = index; return *this; }
		FDescriptorSetLayoutBuilder& SetFlags(vk::DescriptorSetLayoutCreateFlags flags) { mFlags = flags; return *this; }

		FDescriptorSetLayoutBuilder& SetName(FName name) { mName = name; return *this; }

	private:
		std::array<FBinding, kMaxDescriptorsPerSet> mBindings;
		uint16 mNumBindings = 0;
		uint16 mSetIndex = 0;
		vk::DescriptorSetLayoutCreateFlags mFlags = {};

		FName mName;
	};

	class FDescriptorSetBuilder final
	{
		BUILDER_BODY()

	public:
		FDescriptorSetBuilder& Reset() { mNumResources = 0; return *this; }
		FDescriptorSetBuilder& SetLayout(THandle<FDescriptorSetLayout> layout) { mLayout = layout; return *this; }
		FDescriptorSetBuilder& SetDescriptorPool(THandle<FDescriptorPool> descriptorPool) { mDescriptorPool = descriptorPool; return *this; }

		FDescriptorSetBuilder& SetTexture(THandle<FTexture> texture, uint16 binding)
		{
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = texture;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetBuffer(THandle<FBuffer> buffer, uint16 binding)
		{
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = buffer;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetSampler(THandle<FSampler> sampler, uint16 binding)
		{
			mBindings[mNumResources] = binding;
			mResources[mNumResources] = sampler;

			++mNumResources;

			return *this;
		}

		FDescriptorSetBuilder& SetName(FName name) { mName = name; return *this; }
		FDescriptorSetBuilder& SetFlags(vk::DescriptorPoolCreateFlags flags) { mFlags = flags; return *this; }

	private:
		std::array<FHandle, kMaxDescriptorsPerSet> mResources;
		std::array<uint16, kMaxDescriptorsPerSet> mBindings;

		THandle<FDescriptorSetLayout> mLayout = {};
		THandle<FDescriptorPool> mDescriptorPool = {};
		uint32 mNumResources = 0;

		vk::DescriptorPoolCreateFlags mFlags;

		FName mName;
	};

	class FRasterizationBuilder final
	{
		BUILDER_BODY()

	public:
		FRasterizationBuilder& SetCullMode(vk::CullModeFlags cullMode) { mCullMode = cullMode; return *this; }
		FRasterizationBuilder& SetPolygonMode(vk::CullModeFlags polygonMode) { mCullMode = polygonMode; return *this; }
		FRasterizationBuilder& SetFrontFace(vk::FrontFace frontFace) { mFrontFace = frontFace; return *this; }

	private:
		vk::CullModeFlags mCullMode = vk::CullModeFlagBits::eBack;
		vk::PolygonMode mPolygonMode = vk::PolygonMode::eFill;
		vk::FrontFace mFrontFace = vk::FrontFace::eCounterClockwise;
	};

	class FDepthStencilBuilder final
	{
		BUILDER_BODY()

	public:
		FDepthStencilBuilder& SetDepth(bool bTest, bool bWrite, vk::CompareOp compareOperator)
		{
			mbEnableDepthTest = bTest;
			mbEnableWriteDepth = bWrite;
			mDepthCompareOperator = compareOperator;

			return *this;
		}

	private:
		vk::CompareOp mDepthCompareOperator = vk::CompareOp::eGreater;

		bool mbEnableDepthTest : 1 = false;
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

		FBlendStateBuilder& AddNoBlendingState()
		{
			AddBlendState();
			return *this;
		}

	private:
		std::array<FBlendState, kMaxColorAttachments> mBlendStates;
		uint32 mActiveStates = 0;
	};

	struct FShaderStage final
	{
		std::string mShaderName;
		vk::ShaderStageFlagBits mStage;
		std::string mEntryPoint;
	};

	class FPipelineRenderingBuilder final
	{
		BUILDER_BODY()

	public:
		FPipelineRenderingBuilder& Reset() {mNumColorAttachments = 0; return *this; }
		FPipelineRenderingBuilder& SetDepthAttachment(vk::Format format) { mDepthAttachmentFormat = format; return *this;}
		FPipelineRenderingBuilder& AddColorAttachment(vk::Format format)
		{
			mColorAttachmentFormats[mNumColorAttachments] = format;
			++mNumColorAttachments;

			return *this;
		}

	private:
		std::array<vk::Format, kMaxColorAttachments> mColorAttachmentFormats = {};
		uint32 mNumColorAttachments = 0;

		vk::Format mDepthAttachmentFormat = vk::Format::eUndefined;
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
		FRasterizationBuilder& GetRasterization() { return mRasterizationBuilder; }
		FDepthStencilBuilder& GetDepthStencil() { return mDepthStencilBuilder; }
		FBlendStateBuilder& GetBlendState() { return mBlendStateBuilder; }
		FShaderStateBuilder& GetShaderState() { return mShaderStateBuilder; }
		FPipelineRenderingBuilder& GetPipelineRendering() { return mPipelineRenderingBuilder; }

		FPipelineBuilder& AddDescriptorSetLayout(THandle<FDescriptorSetLayout> handle)
			{ mDescriptorSetLayouts[mNumActiveLayouts++] = handle; return *this; }

		template <PushConstant PushConstantType>
		FPipelineBuilder& SetPushConstantType() { mPushConstantSize = sizeof(PushConstantType); return *this; }

		FPipelineBuilder& SetPrimitiveTopology(vk::PrimitiveTopology primitiveTopology) { mTopology = primitiveTopology; return *this; }
		FPipelineBuilder& SetName(FName name) { mName = name; return *this; }

	public:
		FName GetName() const { return mName; }

	private:
		FRasterizationBuilder mRasterizationBuilder;
		FDepthStencilBuilder mDepthStencilBuilder;
		FBlendStateBuilder mBlendStateBuilder;
		FShaderStateBuilder mShaderStateBuilder;
		FPipelineRenderingBuilder mPipelineRenderingBuilder;

		std::array<THandle<FDescriptorSetLayout>, kMaxDescriptorSetLayouts> mDescriptorSetLayouts;
		uint32 mNumActiveLayouts = 1; // The 0th set are always bindless resources

		uint32 mPushConstantSize = 0;

		vk::PrimitiveTopology mTopology = vk::PrimitiveTopology::eTriangleList;

		FName mName = FName();
	};

}

#undef BUILDER_BODY
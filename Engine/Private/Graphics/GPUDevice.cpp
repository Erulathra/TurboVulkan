#include "Graphics/GPUDevice.h"

#include "VkBootstrap.h"
#include "Assets/EngineResources.h"
#include "Core/Engine.h"

#include "Core/Window.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/ShaderCompiler.h"
#include "Graphics/VulkanInitializers.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Turbo
{
	void FGPUDevice::Init(const FGPUDeviceBuilder& gpuDeviceBuilder)
	{
		TURBO_LOG(LogGPUDevice, Info, "Initializing GPU Device.");

		FWindow& window = entt::locator<FWindow>::value();
		window.InitForVulkan();
		window.Init();

		std::vector<cstring> instanceRequiredExtensions = window.GetVulkanRequiredExtensions();

		VULKAN_HPP_DEFAULT_DISPATCHER.init();
		const vkb::Instance builtInstance = CreateVkInstance(instanceRequiredExtensions);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkInstance);

		TURBO_CHECK(window.CreateVulkanSurface(mVkInstance));
		mVkWindowSurface = window.GetVulkanSurface();

		const vkb::PhysicalDevice selectedPhysicalDevice = SelectPhysicalDevice(builtInstance);
		const vkb::Device device = CreateDevice(selectedPhysicalDevice);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkDevice);

		CreateVulkanMemoryAllocator();

		InitializeImmediateCommands();

		EngineResources::InitEngineSamplers();
		EngineResources::InitEngineTextures();

		InitializeBindlessResources();

		IShaderCompiler::Get().Init();

#if WITH_PROFILER
		CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(mImmediateCommandsPool));
		mImmediateCommandsBuffer->Reset();
		mTraceGpuCtx = TRACE_CREATE_GPU_CTX(
			mVkInstance,
			mVkPhysicalDevice,
			mVkDevice,
			mVkGraphicsQueue,
			mImmediateCommandsBuffer.get(),
			VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr,
			VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr
		);
#endif // WITH_PROFILER

		CreateSwapchain();
		CreateFrameDatas();
	}

	void FGPUDevice::InitializeImmediateCommands()
	{
		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		CHECK_VULKAN_RESULT(mImmediateCommandsFence, mVkDevice.createFence(fenceCreateInfo));

		mImmediateCommandsPool = CreateCommandPool(mVkGraphicsQueueFamilyIndex, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		const static FName immediateCommandBuffer = FName("ImmediateGPUCommands");
		mImmediateCommandsBuffer = CreateCommandBuffer(mImmediateCommandsPool, immediateCommandBuffer);
	}

	void FGPUDevice::InitializeBindlessResources()
	{
		TURBO_LOG(LogGPUDevice, Info, "Initializing bindless resources")

		FDescriptorPoolBuilder descriptorPoolBuilder;
		descriptorPoolBuilder
			.SetMaxSets(1)
			.SetPoolRatio(vk::DescriptorType::eSampledImage, kTexturePoolSize)
			.SetPoolRatio(vk::DescriptorType::eStorageImage, kTexturePoolSize)
			.SetPoolRatio(vk::DescriptorType::eSampler, kSamplerPoolSize)
			.SetFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
			.SetName(FName("BindlessResources"));
		mBindlessResourcesPool = CreateDescriptorPool(descriptorPoolBuilder);
		TURBO_CHECK(mBindlessResourcesPool)

		vk::DescriptorBindingFlags bindingFlags = vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound;

		FDescriptorSetLayoutBuilder layoutBuilder;
		layoutBuilder
			.SetIndex(0)
			.SetFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool)
			.AddBinding(vk::DescriptorType::eSampledImage, BindlessResourcesBindings::kSampledImage, kTexturePoolSize, bindingFlags, FName("texturePool"))
			.AddBinding(vk::DescriptorType::eStorageImage, BindlessResourcesBindings::kStorageImage, kTexturePoolSize, bindingFlags, FName("rwTexturePool"))
			.AddBinding(vk::DescriptorType::eSampler, BindlessResourcesBindings::kSampler, kSamplerPoolSize, bindingFlags, FName("samplerPool"))
			.SetName(FName("BindlessResources"));
		mBindlessResourcesLayout = CreateDescriptorSetLayout(layoutBuilder);
		TURBO_CHECK(mBindlessResourcesLayout)

		FDescriptorSetBuilder descriptorSetBuilder;
		descriptorSetBuilder
			.SetDescriptorPool(mBindlessResourcesPool)
			.SetLayout(mBindlessResourcesLayout)
			.SetName(FName("BindlessResources"));

		mBindlessResourcesSet = CreateDescriptorSet(descriptorSetBuilder);

		mBindlessResourcesToUpdate.reserve(kTexturePoolSize);

		for (uint32 textureBindId = 0; textureBindId < kTexturePoolSize; ++textureBindId)
		{
			mBindlessResourcesToUpdate.emplace_back(EResourceType::Texture, textureBindId, EngineResources::GetBlackTexture());
		}
	}

	THandle<FBuffer> FGPUDevice::CreateBuffer(const FBufferBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		const THandle<FBuffer> handle = mBufferPool->Acquire();
		TURBO_CHECK(handle)

#if TURBO_BUILD_DEVELOPMENT
		if (TEST_FLAG(builder.mUsageFlags, vk::BufferUsageFlagBits::eUniformBuffer))
		{
			TURBO_ENSURE(builder.mSize < kMaxUniformBufferSize);
		}
#endif

		FBuffer* buffer = AccessBuffer(handle);
		FBufferCold* bufferCold = AccessBufferCold(handle);
		buffer->mDeviceSize = builder.mSize;

		bufferCold->mName = builder.mName;
		bufferCold->mUsageFlags = builder.mUsageFlags;

		vk::BufferCreateInfo createInfo = {};
		createInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress | bufferCold->mUsageFlags;
		createInfo.size = buffer->mDeviceSize;

		vma::AllocationCreateInfo allocationCreateInfo = {};
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto;

		if (any(builder.mBufferFlags & EBufferFlags::CreateMapped))
		{
			allocationCreateInfo.flags |= vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
			allocationCreateInfo.flags |= vma::AllocationCreateFlagBits::eMapped;
		}

		vma::AllocationInfo allocationInfo;
		std::pair<vk::Buffer, vma::Allocation> allocationResult;
		CHECK_VULKAN_RESULT(allocationResult, mVmaAllocator.createBuffer(createInfo, allocationCreateInfo, allocationInfo));

		buffer->mVkBuffer = allocationResult.first;
		bufferCold->mAllocation = allocationResult.second;

		vk::BufferDeviceAddressInfo deviceAddressInfo = {};
		deviceAddressInfo.buffer = buffer->mVkBuffer;
		buffer->mDeviceAddress = mVkDevice.getBufferAddress(deviceAddressInfo);
		TURBO_CHECK(buffer->mDeviceAddress != 0)

		SetResourceName(buffer->mVkBuffer, bufferCold->mName);

		if (any(builder.mBufferFlags & EBufferFlags::CreateMapped))
		{
			buffer->mMappedAddress = allocationInfo.pMappedData;
		}

		const vk::MemoryPropertyFlags& allocationMemoryProperties = mVmaAllocator.getAllocationMemoryProperties(bufferCold->mAllocation);

		if (builder.mInitialData)
		{
			if (allocationMemoryProperties & vk::MemoryPropertyFlagBits::eHostVisible)
			{
				TRACE_ZONE_SCOPED_N("Copy mapped")

				mVmaAllocator.copyMemoryToAllocation(builder.mInitialData, bufferCold->mAllocation, 0, builder.mSize);
			}
			else // Use staging buffer
			{
				TRACE_ZONE_SCOPED_N("Copy staging buffer")

				ImmediateSubmit(FOnImmediateSubmit::CreateLambda([&](FCommandBuffer& cmd)
				{
					const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(builder.mInitialData, builder.mSize);
					const THandle<FBuffer> stagingBuffer = CreateBuffer(stagingBufferBuilder);

					// ensure that writing to staging buffer are completed before coping it to target buffer;
					cmd.BufferBarrier(
						stagingBuffer,
						vk::AccessFlagBits2::eHostWrite,
						vk::PipelineStageFlagBits2::eHost,
						vk::AccessFlagBits2::eTransferRead,
						vk::PipelineStageFlagBits2::eTransfer
						);

					cmd.CopyBuffer(stagingBuffer, handle, builder.mSize);
					DestroyBuffer(stagingBuffer);

					// No need for synchronization as we would submit this command buffer just after that lambda.
				}));
			}
		}

		return handle;
	}

	THandle<FTexture> FGPUDevice::CreateTexture(const FTextureBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		const THandle<FTexture> handle = mTexturePool->Acquire();
		TURBO_CHECK(handle)

		FTexture* texture = AccessTexture(handle);
		FTextureCold* textureCold = AccessTextureCold(handle);

		InitVulkanTexture(builder, handle, texture, textureCold);

		if (builder.mbBindTexture)
		{
			mBindlessResourcesToUpdate.emplace_back(EResourceType::Texture, handle.GetIndex(), handle);
			texture->mBindIndex = handle.GetIndex();
		}

		return handle;
	}

	THandle<FSampler> FGPUDevice::CreateSampler(const FSamplerBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		const THandle<FSampler> handle = mSamplerPool->Acquire();
		TURBO_CHECK(handle);

		FSampler* sampler = AccessSampler(handle);
		FSamplerCold* samplerCold = AccessSamplerCold(handle);
		samplerCold->mAddressModeU = builder.mAddressModeU;
		samplerCold->mAddressModeV = builder.mAddressModeV;
		samplerCold->mAddressModeW = builder.mAddressModeW;
		samplerCold->mMinFilter = builder.mMinFilter;
		samplerCold->mMagFilter = builder.mMagFilter;
		samplerCold->mMipFilter = builder.mMipFilter;
		samplerCold->mName = builder.mName;

		vk::SamplerCreateInfo createInfo = {};
		createInfo.addressModeU = samplerCold->mAddressModeU;
		createInfo.addressModeV = samplerCold->mAddressModeV;
		createInfo.addressModeW = samplerCold->mAddressModeW;
		createInfo.minFilter = samplerCold->mMinFilter;
		createInfo.magFilter = samplerCold->mMagFilter;
		createInfo.mipmapMode = samplerCold->mMipFilter;
		createInfo.minLod = 0;
		createInfo.maxLod = vk::LodClampNone;

		// TODO:
		createInfo.anisotropyEnable = vk::False;
		createInfo.compareEnable = vk::False;
		createInfo.unnormalizedCoordinates = vk::False;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueWhite;

		CHECK_VULKAN_RESULT(sampler->mVkSampler, mVkDevice.createSampler(createInfo));
		SetResourceName(sampler->mVkSampler, builder.mName);

		mBindlessResourcesToUpdate.emplace_back(EResourceType::Sampler, handle.GetIndex(), handle);

		return handle;
	}

	THandle<FPipeline> FGPUDevice::CreatePipeline(const FPipelineBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		THandle<FPipeline> handle = mPipelinePool->Acquire();
		TURBO_CHECK(handle)

		FPipeline* pipeline = mPipelinePool->Access(handle);
		TURBO_CHECK(pipeline)

		THandle<FShaderState> shaderStateHandle = CreateShaderState(builder.mShaderStateBuilder);
		TURBO_CHECK(shaderStateHandle)

		FShaderState* shaderState = AccessShaderState(shaderStateHandle);
		TURBO_CHECK(shaderState)

		pipeline->mShaderState = shaderStateHandle;
		pipeline->mNumActiveLayouts = builder.mNumActiveLayouts;
		pipeline->mbGraphicsPipeline = shaderState->mbGraphicsPipeline;

		std::array<vk::DescriptorSetLayout, kMaxDescriptorSetLayouts> vkLayouts;

		// Bind bindless descriptor set layout
		pipeline->mDescriptorLayoutsHandles[0] = mBindlessResourcesLayout;
		const FDescriptorSetLayout* bindlessSetLayout = mDescriptorSetLayoutPool->Access(mBindlessResourcesLayout);
		vkLayouts[0] = bindlessSetLayout->mVkLayout;

		// Bind rest of the descriptors
		for (uint32 layoutId = 1; layoutId < builder.mNumActiveLayouts; ++layoutId)
		{
			const THandle<FDescriptorSetLayout> setLayoutHandle = builder.mDescriptorSetLayouts[layoutId];
			pipeline->mDescriptorLayoutsHandles[layoutId] = setLayoutHandle;
			const FDescriptorSetLayout* setLayout = mDescriptorSetLayoutPool->Access(setLayoutHandle);
			vkLayouts[layoutId] = setLayout->mVkLayout;
		}

		vk::PushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = builder.mPushConstantSize;

		if (shaderState->mbGraphicsPipeline)
		{
			pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
		}
		else
		{
			pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
		}

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.pSetLayouts = vkLayouts.data();
		pipelineLayoutCreateInfo.setLayoutCount = builder.mNumActiveLayouts;
		pipelineLayoutCreateInfo.setPushConstantRanges({pushConstantRange});

		CHECK_VULKAN_RESULT(pipeline->mVkLayout, mVkDevice.createPipelineLayout(pipelineLayoutCreateInfo))

		if (shaderState->mbGraphicsPipeline)
		{
			vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> chain;
			vk::GraphicsPipelineCreateInfo& pipelineCreateInfo = chain.get<vk::GraphicsPipelineCreateInfo>();
			pipelineCreateInfo.pStages = shaderState->mShaderStageCrateInfo.data();
			pipelineCreateInfo.stageCount = shaderState->mNumActiveShaders;
			pipelineCreateInfo.layout = pipeline->mVkLayout;

			// Vertex input
			constexpr vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
			pipelineCreateInfo.pVertexInputState = &vertexInputState;

			// Input Assembly
			vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
			inputAssemblyState.topology = builder.mTopology;
			inputAssemblyState.primitiveRestartEnable = vk::False;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;

			// Color blending
			std::array<vk::PipelineColorBlendAttachmentState, 8> colorBlendAttachments;
			if (builder.mBlendStateBuilder.mActiveStates > 0)
			{
				for (uint32 stateId = 0; stateId < builder.mBlendStateBuilder.mActiveStates; ++stateId)
				{
					const FBlendState& blendState = builder.mBlendStateBuilder.mBlendStates[stateId];
					vk::PipelineColorBlendAttachmentState& attachment = colorBlendAttachments[stateId];

					attachment.colorWriteMask = kRGBABits;
					attachment.blendEnable = blendState.mbBlendEnabled ? vk::True : vk::False;

					attachment.srcColorBlendFactor = blendState.mSourceColorBlendFactor;
					attachment.dstColorBlendFactor = blendState.mDestinationColorBlendFactor;
					attachment.colorBlendOp = blendState.mColorBlendOperator;

					attachment.srcColorBlendFactor = blendState.mSourceAlphaBlendFactor;
					attachment.dstColorBlendFactor = blendState.mDestinationAlphaBlendFactor;
					attachment.colorBlendOp = blendState.mAlphaBlendOperator;
				}
			}
			else
			{
				colorBlendAttachments[0] = vk::PipelineColorBlendAttachmentState();
				colorBlendAttachments[0].blendEnable = vk::False;
				colorBlendAttachments[0].colorWriteMask = kRGBABits;
			}

			vk::PipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.logicOpEnable = vk::False;
			colorBlending.attachmentCount = builder.mBlendStateBuilder.mActiveStates;
			colorBlending.pAttachments = colorBlendAttachments.data();

			TURBO_CHECK_MSG(builder.mBlendStateBuilder.mActiveStates > 0, "It must be at least 1 color attachment.")

			pipelineCreateInfo.pColorBlendState = &colorBlending;

			// Depth Stencil
			// TODO: better depht handling
			vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.depthWriteEnable = builder.mDepthStencilBuilder.mbEnableWriteDepth ? vk::True : vk::False;
			depthStencil.depthTestEnable = builder.mDepthStencilBuilder.mbEnableDepthTest ? vk::True : vk::False;
			depthStencil.stencilTestEnable = builder.mDepthStencilBuilder.mbEnableStencil ? vk::True : vk::False;
			depthStencil.depthCompareOp = builder.mDepthStencilBuilder.mDepthCompareOperator;
			depthStencil.minDepthBounds = 0.f;
			depthStencil.maxDepthBounds = 1.f;

			if (builder.mDepthStencilBuilder.mbEnableStencil)
			{
				TURBO_UNINPLEMENTED()
			}

			pipelineCreateInfo.pDepthStencilState = &depthStencil;

			// Multi sample (unimplemented)
			vk::PipelineMultisampleStateCreateInfo multisampleState = {};
			multisampleState.sampleShadingEnable = vk::False;
			multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

			pipelineCreateInfo.pMultisampleState = &multisampleState;

			// Rasterizer state
			vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
			rasterizationState.depthClampEnable = vk::False;
			rasterizationState.rasterizerDiscardEnable = vk::False;
			rasterizationState.polygonMode = vk::PolygonMode::eFill;
			rasterizationState.lineWidth = 1.f;
			rasterizationState.cullMode = builder.mRasterizationBuilder.mCullMode;
			rasterizationState.frontFace = builder.mRasterizationBuilder.mFrontFace;
			rasterizationState.depthBiasEnable = vk::False;
			pipelineCreateInfo.pRasterizationState = &rasterizationState;

			// Tesselation (unimplemented)
			pipelineCreateInfo.pTessellationState = nullptr;

			// Viewport state (default, overridden by dynamic state)
			vk::PipelineViewportStateCreateInfo viewportState = {};
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;
			pipelineCreateInfo.pViewportState = &viewportState;

			// Dynamic states
			std::array dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
			vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
			dynamicStateCreateInfo.setDynamicStates(dynamicStates);

			pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

			vk::PipelineRenderingCreateInfo& pipelineRendering = chain.get<vk::PipelineRenderingCreateInfo>();
			pipelineRendering.colorAttachmentCount = builder.mPipelineRenderingBuilder.mNumColorAttachments;
			pipelineRendering.pColorAttachmentFormats = builder.mPipelineRenderingBuilder.mColorAttachmentFormats.data();
			pipelineRendering.depthAttachmentFormat = builder.mPipelineRenderingBuilder.mDepthAttachmentFormat;

			TURBO_CHECK_MSG(builder.mBlendStateBuilder.mActiveStates == builder.mPipelineRenderingBuilder.mNumColorAttachments, "Blend states number must be the same as num color attachments.")
			TURBO_CHECK(builder.mPipelineRenderingBuilder.mNumColorAttachments > 0)

			CHECK_VULKAN_RESULT(pipeline->mVkPipeline, mVkDevice.createGraphicsPipeline(nullptr, pipelineCreateInfo));
			pipeline->mVkBindPoint = vk::PipelineBindPoint::eGraphics;
		}
		else
		{
			vk::ComputePipelineCreateInfo pipelineCreateInfo = {};
			pipelineCreateInfo.stage = shaderState->mShaderStageCrateInfo[0];
			pipelineCreateInfo.layout = pipeline->mVkLayout;

			CHECK_VULKAN_RESULT(pipeline->mVkPipeline, mVkDevice.createComputePipeline(nullptr, pipelineCreateInfo));
			pipeline->mVkBindPoint = vk::PipelineBindPoint::eCompute;
		}

		SetResourceName(pipeline->mVkPipeline, builder.mName);

		return handle;
	}

	THandle<FDescriptorPool> FGPUDevice::CreateDescriptorPool(const FDescriptorPoolBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		THandle<FDescriptorPool> handle = mDescriptorPoolPool->Acquire();
		TURBO_CHECK(handle)

		FDescriptorPool* pool = mDescriptorPoolPool->Access(handle);
		pool->mDescriptorSets.clear();
		pool->mName = builder.mName;

		std::vector<vk::DescriptorPoolSize> poolSizes;
		poolSizes.reserve(builder.mPoolSizes.size());
		for (auto [type, ratio] : builder.mPoolSizes)
		{
			poolSizes.emplace_back(type, static_cast<uint32>(ratio * builder.mMaxSets));
		}

		vk::DescriptorPoolCreateInfo createInfo = {};
		createInfo.flags = builder.mFlags;
		createInfo.maxSets = builder.mMaxSets;
		createInfo.setPoolSizes(poolSizes);

		CHECK_VULKAN_RESULT(pool->mVkDescriptorPool, mVkDevice.createDescriptorPool(createInfo));
		SetResourceName(pool->mVkDescriptorPool, pool->mName);

		return handle;
	}

	THandle<FDescriptorSetLayout> FGPUDevice::CreateDescriptorSetLayout(const FDescriptorSetLayoutBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		THandle<FDescriptorSetLayout> handle = mDescriptorSetLayoutPool->Acquire();
		TURBO_CHECK(handle)

		FDescriptorSetLayout* layout = mDescriptorSetLayoutPool->Access(handle);
		layout->mNumBindings = builder.mNumBindings;
		layout->mHandle = handle;
		layout->mSetIndex = builder.mSetIndex;

		std::array<vk::DescriptorBindingFlags, kMaxDescriptorsPerSet> bindingFlags;

		for (uint32 bindingId = 0; bindingId < builder.mNumBindings; ++bindingId)
		{
			const FBinding& builderBinding = builder.mBindings[bindingId];
			bindingFlags[bindingId] = builderBinding.mFlags;
			layout->mBindings[bindingId] = builderBinding;

			vk::DescriptorSetLayoutBinding& vkBinding = layout->mVkBindings[bindingId];
			vkBinding = vk::DescriptorSetLayoutBinding();
			vkBinding.binding = builderBinding.mIndex;
			vkBinding.descriptorType = builderBinding.mType;
			vkBinding.descriptorCount = builderBinding.mCount;

			vkBinding.stageFlags = vk::ShaderStageFlagBits::eAll;
		}

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.pBindings = layout->mVkBindings->data();
		layoutCreateInfo.bindingCount = builder.mNumBindings;
		layoutCreateInfo.flags = builder.mFlags;

		vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
		bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();
		bindingFlagsCreateInfo.bindingCount = builder.mNumBindings;

		layoutCreateInfo.pNext = bindingFlagsCreateInfo;

		CHECK_VULKAN_RESULT(layout->mVkLayout, mVkDevice.createDescriptorSetLayout(layoutCreateInfo));

		return handle;
	}

	THandle<FDescriptorSet> FGPUDevice::CreateDescriptorSet(const FDescriptorSetBuilder& builder)
	{
		THandle<FDescriptorSet> handle = mDescriptorSetPool->Acquire();
		TURBO_CHECK(handle);

		FDescriptorSet* set = mDescriptorSetPool->Access(handle);
		const FDescriptorSetLayout* layout = mDescriptorSetLayoutPool->Access(builder.mLayout);
		TURBO_CHECK(set && layout)

		FDescriptorPool* pool = mDescriptorPoolPool->Access(builder.mDescriptorPool);

		// Allocate set
		vk::DescriptorSetAllocateInfo allocateInfo = {};
		allocateInfo.descriptorPool = pool->mVkDescriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.setSetLayouts({layout->mVkLayout});

		std::vector<vk::DescriptorSet> descriptorSets;
		CHECK_VULKAN_RESULT(descriptorSets, mVkDevice.allocateDescriptorSets(allocateInfo))

		set->mVkDescriptorSet = descriptorSets.front();
		set->mOwnerPool = builder.mDescriptorPool;

		pool->mDescriptorSets.push_back(handle);

		std::array<vk::WriteDescriptorSet, kMaxDescriptorsPerSet> writes;

		std::vector<vk::DescriptorImageInfo> imageInfos;
		imageInfos.reserve(kMaxDescriptorsPerSet);
		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		bufferInfos.reserve(kMaxDescriptorsPerSet);

		uint32 numWrites = 0;

		for (uint32 bindingId = 0; bindingId < layout->mNumBindings; ++bindingId)
		{
			const FBinding& binding = layout->mBindings[bindingId];
			const FHandle resource = builder.mResources[bindingId];

			const bool bPartiallyBound = TEST_FLAG(binding.mFlags, vk::DescriptorBindingFlagBits::ePartiallyBound);

			TURBO_CHECK(resource.IsValid() || bPartiallyBound)

			if (resource.IsValid())
			{
				vk::WriteDescriptorSet& write = writes[numWrites];
				write.descriptorCount = 1;
				write.dstSet = set->mVkDescriptorSet;
				write.dstBinding = bindingId;
				write.descriptorType = binding.mType;

				++numWrites;

				switch (binding.mType)
				{
				case vk::DescriptorType::eSampledImage:
				case vk::DescriptorType::eStorageImage:
					{
						const FTexture* texture = AccessTexture(THandle<FTexture>(resource));
						const FTextureCold* textureCold = AccessTextureCold(THandle<FTexture>(resource));

						vk::DescriptorImageInfo& imageInfo = imageInfos.emplace_back();
						imageInfo.imageView = texture->mVkImageView;

						if (binding.mType == vk::DescriptorType::eSampledImage)
						{
							imageInfo.imageLayout =
								TextureFormat::HasDepthOrStencil(textureCold->mFormat)
									? vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal
									: vk::ImageLayout::eReadOnlyOptimal;
						}
						else
						{
							imageInfo.imageLayout = vk::ImageLayout::eGeneral;
						}

						write.setImageInfo({imageInfo});
						break;
					}
				case vk::DescriptorType::eSampler:
					{
						const FSampler* sampler = mSamplerPool->Access(THandle<FSampler>(resource));

						vk::DescriptorImageInfo& imageInfo = imageInfos.emplace_back();
						imageInfo.sampler = sampler->mVkSampler;

						write.setImageInfo({imageInfo});
						break;
					}
				case vk::DescriptorType::eStorageBuffer:
				case vk::DescriptorType::eUniformBuffer:
					{
						const FBuffer* buffer = mBufferPool->Access(THandle<FBuffer>(resource));

						vk::DescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
						bufferInfo.buffer = buffer->mVkBuffer;
						bufferInfo.offset = 0;
						bufferInfo.range = buffer->mDeviceSize;

						write.pBufferInfo = &bufferInfo;
						break;
					}
				default:
					TURBO_UNINPLEMENTED()
				}
			}
		}

		mVkDevice.updateDescriptorSets(numWrites, writes.data(), 0, nullptr);

		return handle;
	}

	THandle<FShaderState> FGPUDevice::CreateShaderState(const FShaderStateBuilder& builder)
	{
		TRACE_ZONE_SCOPED()

		THandle<FShaderState> handle = {};

		if (builder.mStagesCount == 0)
		{
			TURBO_LOG(LogGPUDevice, Warn, "Shader {} doesn't contain any shader stage.", builder.mName);
			return handle;
		}

		handle = mShaderStatePool->Acquire();
		TURBO_CHECK(handle)

		FShaderState* shaderState = mShaderStatePool->Access(handle);
		TURBO_CHECK(shaderState)

		shaderState->mShaderStageCrateInfo = {};

		shaderState->mbGraphicsPipeline = true;
		shaderState->mNumActiveShaders = 0;

		if (builder.mStages[0].mStage == vk::ShaderStageFlagBits::eCompute)
		{
			TURBO_CHECK_MSG(builder.mStagesCount == 1, "ComputePipeline supports only 1 compute shader.")
			shaderState->mbGraphicsPipeline = false;
		}

		std::unordered_set<vk::ShaderStageFlagBits> processedStages;

		IShaderCompiler& shaderCompiler = IShaderCompiler::Get();
		for (uint32 shaderStageId = 0; shaderStageId < builder.mStagesCount; ++shaderStageId)
		{
			const FShaderStage& shaderStage = builder.mStages[shaderStageId];

			auto insertionResult = processedStages.insert(shaderStage.mStage);
			TURBO_CHECK_MSG(insertionResult.second, "Only one shader per stage is supported.")

			const vk::ShaderModule shaderModule = shaderCompiler.CompileShader(mVkDevice, shaderStage);
			TURBO_CHECK(shaderModule);

			SetResourceName(shaderModule, shaderStage.mShaderName);

			vk::PipelineShaderStageCreateInfo& stageCreateInfo = shaderState->mShaderStageCrateInfo[shaderStageId];
			*stageCreateInfo = vk::PipelineShaderStageCreateInfo();
			stageCreateInfo.pName = "main";
			stageCreateInfo.setStage(shaderStage.mStage);
			stageCreateInfo.module = shaderModule;
		}

		shaderState->mNumActiveShaders = builder.mStagesCount;
		shaderState->mName = builder.mName;

		return handle;
	}

	void FGPUDevice::ResetDescriptorPool(THandle<FDescriptorPool> descriptorPoolHandle)
	{
		TRACE_ZONE_SCOPED()

		FDescriptorPool* descriptorPool = AccessDescriptorPool(descriptorPoolHandle);
		TURBO_CHECK(descriptorPool)

		mVkDevice.resetDescriptorPool(descriptorPool->mVkDescriptorPool);

		for (const THandle<FDescriptorSet>& descriptorSet : descriptorPool->mDescriptorSets)
		{
			mDescriptorSetPool->Release(descriptorSet);
		}

		descriptorPool->mDescriptorSets.clear();
	}

	void FGPUDevice::DestroyBuffer(THandle<FBuffer> handle)
	{
		const FBuffer* buffer = AccessBuffer(handle);
		const FBufferCold* bufferCold = AccessBufferCold(handle);
		TURBO_CHECK(buffer);

		TURBO_LOG(LogGPUDevice, Display, "Destroying {} buffer.", bufferCold->mName);

		FBufferDestroyer destroyer;
		destroyer.mHandle = handle;
		destroyer.mVkBuffer = buffer->mVkBuffer;
		destroyer.mAllocation = bufferCold->mAllocation;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	void FGPUDevice::DestroyTexture(THandle<FTexture> handle)
	{
		const FTexture* texture = AccessTexture(handle);
		TURBO_CHECK(texture)

#if TURBO_BUILD_DEVELOPMENT
		const FTextureCold* textureCold = AccessTextureCold(handle);
		TURBO_LOG(LogGPUDevice, Display, "Destroying {} texture.", textureCold->mName);
#endif // TURBO_BUILD_DEVELOPMENT

		FTextureDestroyer destroyer = {};
		destroyer.mHandle = handle;
		destroyer.mImage = texture->mVkImage;
		destroyer.mImageView = texture->mVkImageView;
		destroyer.mImageAllocation = texture->mImageAllocation;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	void FGPUDevice::DestroySampler(THandle<FSampler> handle)
	{
		const FSampler* sampler = AccessSampler(handle);
		TURBO_CHECK(sampler)

#if TURBO_BUILD_DEVELOPMENT
		const FSamplerCold* samplerCold = AccessSamplerCold(handle);
		TURBO_LOG(LogGPUDevice, Display, "Destroying {} sampler.", samplerCold->mName);
#endif // TURBO_BUILD_DEVELOPMENT

		FSamplerDestroyer destroyer = {};
		destroyer.mHandle = handle;
		destroyer.mVkSampler = sampler->mVkSampler;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	void FGPUDevice::DestroyPipeline(THandle<FPipeline> handle)
	{
		const FPipeline* pipeline = AccessPipeline(handle);
		TURBO_CHECK(pipeline);

		FPipelineDestroyer destroyer = {};
		destroyer.mPipeline = pipeline->mVkPipeline;
		destroyer.mLayout = pipeline->mVkLayout;
		destroyer.mHandle = handle;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);

		DestroyShaderState(pipeline->mShaderState);
	}

	void FGPUDevice::DestroyDescriptorPool(THandle<FDescriptorPool> handle)
	{
		const FDescriptorPool* descriptorPool = AccessDescriptorPool(handle);
		TURBO_CHECK(descriptorPool)

		FDescriptorPoolDestroyer destroyer = {};
		destroyer.mVkDescriptorPool = descriptorPool->mVkDescriptorPool;
		destroyer.mhandle = handle;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	void FGPUDevice::DestroyDescriptorSetLayout(THandle<FDescriptorSetLayout> handle)
	{
		const FDescriptorSetLayout* layout = AccessDescriptorSetLayout(handle);
		TURBO_CHECK(layout)

		FDescriptorSetLayoutDestroyer destroyer = {};
		destroyer.mVkLayout = layout->mVkLayout;
		destroyer.mHandle = layout->mHandle;

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	void FGPUDevice::DestroyShaderState(THandle<FShaderState> handle)
	{
		const FShaderState* shaderState = AccessShaderState(handle);
		TURBO_CHECK(shaderState)

		FShaderStateDestroyer destroyer = {};
		destroyer.mHandle = handle;
		destroyer.mNumActiveShaders = shaderState->mNumActiveShaders;

		for (uint32 shaderId = 0; shaderId < shaderState->mNumActiveShaders; ++shaderId)
		{
			destroyer.mModules[shaderId] = shaderState->mShaderStageCrateInfo[shaderId].module;
		}

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		frameData.mDestroyQueue.RequestDestroy(destroyer);
	}

	vkb::Instance FGPUDevice::CreateVkInstance(const std::vector<cstring>& requiredExtensions)
	{
		// Copy by design
		std::vector<cstring> enableExtensions = requiredExtensions;

#if WITH_DEBUG_RENDERING
		enableExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // WITH_DEBUG_RENDERING

		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name("TurboEngine")
			.set_app_version(TURBO_VERSION())
			.enable_extensions(enableExtensions)
#if WITH_VALIDATION_LAYERS
			.request_validation_layers(true)
			.set_debug_callback(&FGPUDevice::ValidationLayerCallback)
			// .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
#endif // WITH_VALIDATION_LAYERS
			.require_api_version(kVulkanVersion);


		vkb::Result<vkb::Instance> buildInstanceResult = instanceBuilder.build();
		TURBO_CHECK_MSG(buildInstanceResult, "Vulkan Instance Creation failed. Reason: {}", buildInstanceResult.error().message())

		mVkInstance = buildInstanceResult.value();
		mVkDebugUtilsMessenger = buildInstanceResult.value().debug_messenger;

		return buildInstanceResult.value();
	}

	vkb::PhysicalDevice FGPUDevice::SelectPhysicalDevice(const vkb::Instance& builtInstance)
	{
		TURBO_CHECK(mVkWindowSurface)

		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.setTextureCompressionBC(true);
		deviceFeatures.setShaderFloat64(true);
		deviceFeatures.setShaderInt64(true);

		vk::PhysicalDeviceVulkan11Features device11Features = {};
		device11Features.setShaderDrawParameters(true);

		vk::PhysicalDeviceVulkan12Features device12Features = {};
		device12Features.setBufferDeviceAddress(true);
		device12Features.setDescriptorIndexing(true);
		device12Features.setDescriptorBindingPartiallyBound(true);
		device12Features.setRuntimeDescriptorArray(true);
		device12Features.setScalarBlockLayout(true);
		device12Features.setDescriptorBindingSampledImageUpdateAfterBind(true);
		device12Features.setDescriptorBindingStorageImageUpdateAfterBind(true);

		vk::PhysicalDeviceVulkan13Features device13Features = {};
		device13Features.setDynamicRendering(true);
		device13Features.setSynchronization2(true);

		vkb::PhysicalDeviceSelector physicalDeviceSelector(builtInstance);
		physicalDeviceSelector
			.set_surface(mVkWindowSurface)
			.require_present(true)
			.set_required_features(deviceFeatures)
			.set_required_features_11(device11Features)
			.set_required_features_12(device12Features)
			.set_required_features_13(device13Features)
			.set_minimum_version(1, 3)
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vkb::Result<vkb::PhysicalDevice> selectPhysicalDeviceResult = physicalDeviceSelector.select();
		TURBO_CHECK_MSG(selectPhysicalDeviceResult, "Physical device selection failed. Reason: {}", selectPhysicalDeviceResult.error().message())

		vkb::PhysicalDevice physicalDevice = selectPhysicalDeviceResult.value();

		mVkPhysicalDevice = physicalDevice;
		TURBO_LOG(LogGPUDevice, Info, "Selected {} as primary physical device.", physicalDevice.name);

		return physicalDevice;
	}

	vkb::Device FGPUDevice::CreateDevice(const vkb::PhysicalDevice& physicalDevice)
	{
		vkb::DeviceBuilder deviceBuilder(physicalDevice);
		vkb::Result<vkb::Device> buildDeviceResult = deviceBuilder.build();
		TURBO_CHECK_MSG(buildDeviceResult, "Device creation failed. Reason: {}", buildDeviceResult.error().message())

		mVkDevice = buildDeviceResult.value();

		auto GetQueue = [&](vkb::QueueType queueType, vk::Queue& OutQueue, uint32& OutFamilyIndex)
		{
			vkb::Result<VkQueue> getQueueResult = buildDeviceResult->get_dedicated_queue(queueType);
			if (getQueueResult.has_value() == false)
			{
				getQueueResult = buildDeviceResult->get_queue(queueType);
			}
			TURBO_CHECK_MSG(getQueueResult, "{} Queue query failed. Reason: {}", magic_enum::enum_name(queueType), getQueueResult.error().message())
			OutQueue = getQueueResult.value();

			vkb::Result<uint32> getQueueIndexResult = buildDeviceResult->get_dedicated_queue_index(queueType);
			if (getQueueIndexResult.has_value() == false)
			{
				getQueueIndexResult = buildDeviceResult->get_queue_index(queueType);
			}
			TURBO_CHECK_MSG(getQueueIndexResult, "{} Queue family query failed. Reason: {}", magic_enum::enum_name(queueType), getQueueIndexResult.error().message())
			OutFamilyIndex = getQueueIndexResult.value();
		};

		GetQueue(vkb::QueueType::graphics, mVkGraphicsQueue, mVkGraphicsQueueFamilyIndex);
		GetQueue(vkb::QueueType::compute, mVkComputeQueue, mVkComputeQueueFamilyIndex);
		GetQueue(vkb::QueueType::transfer, mVkTransferQueue, mVkTransferQueueFamilyIndex);

		const uint32 presentQueueFamily = buildDeviceResult->get_queue_index(vkb::QueueType::present).value();

		TURBO_CHECK_MSG(
			mVkGraphicsQueueFamilyIndex == presentQueueFamily,
			"Graphics queue family ({}) is different than present queue family ({}).",
			mVkGraphicsQueueFamilyIndex,
			presentQueueFamily
		)

		return buildDeviceResult.value();
	}

	std::array<FName, kMaxSwapChainImages> CreateSwapChainTexturesNames()
	{
		std::array<FName, kMaxSwapChainImages> result;
		for (uint32 textureId = 0; textureId < result.size(); ++textureId)
		{
			result[textureId] = FName(fmt::format("SwapchainTexture_{}", textureId));
		}

		return result;
	}

	vkb::Swapchain FGPUDevice::CreateSwapchain()
	{
		TURBO_CHECK(mVkWindowSurface)

		const FWindow& window = entt::locator<FWindow>::value();
		const glm::ivec2& frameBufferSize = window.GetFrameBufferSize();
		TURBO_LOG(LogGPUDevice, Info, "Creating swapchain of size: {}", frameBufferSize);

		vkb::SwapchainBuilder swapchainBuilder {mVkPhysicalDevice, mVkDevice, mVkWindowSurface};
		vkb::Result<vkb::Swapchain> buildSwapchainResult = swapchainBuilder
			.set_desired_present_mode(static_cast<VkPresentModeKHR>(GetBestPresentMode()))
			.set_desired_format(vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
			.set_desired_extent(frameBufferSize.x, frameBufferSize.y)
			.add_image_usage_flags( static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferDst))
			.build();

		TURBO_CHECK_MSG(buildSwapchainResult, "Cannot construct Swapchain. Reason: {}", buildSwapchainResult.error().message())

		vkb::Swapchain builtSwapchain = buildSwapchainResult.value();
		mVkSwapchain = builtSwapchain;
		mVkSurfaceFormat = { builtSwapchain.image_format, builtSwapchain.color_space };
		mPresentMode = static_cast<vk::PresentModeKHR>(builtSwapchain.present_mode);

		TURBO_LOG(LogGPUDevice, Info, "Selected present mode: {}", magic_enum::enum_name(mPresentMode));

		const std::vector<VkImage> builtImages = builtSwapchain.get_images().value();
		const std::vector<VkImageView> builtImageViews = builtSwapchain.get_image_views().value();

		vk::SemaphoreCreateInfo semaphoreCreateInfo = VkInit::SemaphoreCreateInfo();

		mNumSwapChainImages = builtSwapchain.image_count;
		TURBO_CHECK(mNumSwapChainImages <= kMaxSwapChainImages);

		for (uint32 imageId = 0; imageId < mNumSwapChainImages; ++imageId)
		{
			THandle<FTexture> handle = mTexturePool->Acquire();
			FTexture* texture = mTexturePool->Access(handle);
			FTextureCold* textureCold = mTexturePool->AccessCold(handle);
			*texture = {};
			texture->mVkImage = builtImages[imageId];
			texture->mVkImageView = builtImageViews[imageId];

			textureCold->mFormat = mVkSurfaceFormat.format;

			textureCold->mWidth = frameBufferSize.x;
			textureCold->mHeight = frameBufferSize.y;

			textureCold->mHandle = handle;

			static const std::array<FName, kMaxSwapChainImages> swapChainTextureNames = CreateSwapChainTexturesNames();
			textureCold->mName = swapChainTextureNames[imageId];
			SetResourceName(texture->mVkImage, textureCold->mName);
			SetResourceName(texture->mVkImageView, textureCold->mName);

			mSwapChainTextures[imageId] = handle;

			CHECK_VULKAN_RESULT(mSubmitSemaphores[imageId], mVkDevice.createSemaphore(semaphoreCreateInfo));
		}

		return builtSwapchain;
	}

	void FGPUDevice::CreateVulkanMemoryAllocator()
	{
		vma::AllocatorCreateInfo createInfo = {};
		createInfo.setPhysicalDevice(mVkPhysicalDevice);
		createInfo.setDevice(mVkDevice);
		createInfo.setInstance(mVkInstance);

		createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

		vma::VulkanFunctions vulkanFunctions{};
		vulkanFunctions.setVkGetInstanceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr);
		vulkanFunctions.setVkGetDeviceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);

		createInfo.pVulkanFunctions = &vulkanFunctions;

		CHECK_VULKAN_RESULT(mVmaAllocator, vma::createAllocator(createInfo));
	}

	void FGPUDevice::CreateFrameDatas()
	{
		TURBO_LOG(LogGPUDevice, Info, "Creating frames data")

		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();

		FDescriptorPoolBuilder descriptorPoolBuilder;
		descriptorPoolBuilder.SetMaxSets(128);

		for (uint32 frameDataId = 0; frameDataId < mFrameDatas.size(); ++frameDataId)
		{
			FBufferedFrameData& frameData = mFrameDatas[frameDataId];

			CHECK_VULKAN_RESULT(frameData.mCommandBufferExecutedFence, mVkDevice.createFence(fenceCreateInfo));
			CHECK_VULKAN_RESULT(frameData.mImageAcquiredSemaphore, mVkDevice.createSemaphore(semaphoreCreateInfo));

			frameData.mCommandPool = CreateCommandPool(mVkGraphicsQueueFamilyIndex);
			frameData.mCommandBuffer = CreateCommandBuffer(frameData.mCommandPool, FName(fmt::format("Frame{}", frameDataId)));

			frameData.mDescriptorPoolHandle = CreateDescriptorPool(descriptorPoolBuilder);
		}
	}

	vk::CommandPool FGPUDevice::CreateCommandPool(uint32 queueFamilyIndex, vk::CommandPoolCreateFlags createFlags)
	{
		vk::CommandPoolCreateInfo createInfo = {};
		createInfo.setQueueFamilyIndex(queueFamilyIndex);
		createInfo.setFlags(createFlags);

		vk::CommandPool pool;
		CHECK_VULKAN_RESULT(pool, mVkDevice.createCommandPool(createInfo));

		return pool;
	}

	TUniquePtr<FCommandBuffer> FGPUDevice::CreateCommandBuffer(vk::CommandPool commandPool, FName name)
	{
		vk::CommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setCommandBufferCount(1);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

		std::vector<vk::CommandBuffer> commandBuffers;
		CHECK_VULKAN_RESULT(commandBuffers, mVkDevice.allocateCommandBuffers(allocateInfo));

		TUniquePtr<FCommandBuffer> result = std::make_unique<FCommandBuffer>();
		result->mGpu = this;
		result->mVkCommandBuffer = commandBuffers.front();
		result->Reset();

		if (!name.IsNone())
		{
			SetResourceName(result->mVkCommandBuffer, name);
		}

		return result;
	}

	vk::PresentModeKHR FGPUDevice::GetBestPresentMode()
	{
		TURBO_CHECK(mVkWindowSurface)

		std::vector<vk::PresentModeKHR> supportedModes;
		CHECK_VULKAN_RESULT(supportedModes, mVkPhysicalDevice.getSurfacePresentModesKHR(mVkWindowSurface));

		if (mbVSync)
		{
			if (std::ranges::find(supportedModes, vk::PresentModeKHR::eImmediate) != supportedModes.end())
			{
				return vk::PresentModeKHR::eImmediate;
			}
		}

		if (std::ranges::find(supportedModes, vk::PresentModeKHR::eMailbox) != supportedModes.end())
		{
			return vk::PresentModeKHR::eMailbox;
		}

		return vk::PresentModeKHR::eFifo;
	}

	void FGPUDevice::ResizeSwapChain()
	{
		WaitIdle();

		DestroySwapChain();
		CreateSwapchain();

		const FWindow& window = entt::locator<FWindow>::value();
		const glm::ivec2& frameBufferSize = window.GetFrameBufferSize();
		entt::locator<FGeometryBuffer>::value().Resize(frameBufferSize);

		mbRequestedSwapchainResize = false;
	}

	void FGPUDevice::Shutdown()
	{
		CHECK_VULKAN_HPP(mVkDevice.waitIdle())

		TURBO_LOG(LogGPUDevice, Info, "Starting Gpu Device shutdown.")

		DestroyBindlessResources();
		DestroyImmediateCommands();
		DestroyFrameDatas();
		DestroySwapChain();

		IShaderCompiler::Get().Destroy();

#if WITH_PROFILER
		if (mTraceGpuCtx)
		{
			TRACE_DESTROY_GPU_CTX(mTraceGpuCtx);
		}
#endif // WITH_PROFILER

		if (mVmaAllocator)
		{
			mVmaAllocator.destroy();
		}

		if (mVkDevice)
		{
			mVkDevice.destroy();
		}

		entt::locator<FWindow>::value().DestroyVulkanSurface(mVkInstance);

		if (mVkInstance)
		{
#if TURBO_BUILD_DEVELOPMENT
			mVkInstance.destroyDebugUtilsMessengerEXT(mVkDebugUtilsMessenger);
#endif // TURBO_BUILD_SHIPPING
			mVkInstance.destroy();
		}
	}

	bool FGPUDevice::BeginFrame()
	{
		TRACE_ZONE_SCOPED()

		if (mbRequestedSwapchainResize)
		{
			ResizeSwapChain();
		}

		FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];

		// Wait for previous frame fence, and reset it
		const vk::Fence renderCompleteFence = frameData.mCommandBufferExecutedFence;
		CHECK_VULKAN_HPP(mVkDevice.waitForFences({renderCompleteFence}, vk::True, kMaxTimeout));

		frameData.mDestroyQueue.Flush(*this);

		// Acquire next swapchain image
		const vk::Semaphore imageAcquiredSemaphore = frameData.mImageAcquiredSemaphore;

		vk::Result acquireImageResult;
		std::tie(acquireImageResult, mCurrentSwapchainImageIndex) =
			mVkDevice.acquireNextImageKHR(mVkSwapchain, kMaxTimeout, imageAcquiredSemaphore, nullptr);

		if (acquireImageResult == vk::Result::eErrorOutOfDateKHR)
		{
			mbRequestedSwapchainResize = true;
			return false;
		}

		CHECK_VULKAN_HPP(mVkDevice.resetFences({renderCompleteFence}));

		CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(frameData.mCommandPool));
		frameData.mCommandBuffer->Reset();
		frameData.mCommandBuffer->Begin();

		ResetDescriptorPool(frameData.mDescriptorPoolHandle);

		return true;
	}

	bool FGPUDevice::PresentFrame()
	{
		TRACE_ZONE_SCOPED()

		const FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];
		const THandle<FTexture> swapChainTexture = mSwapChainTextures[mCurrentSwapchainImageIndex];

		TRACE_GPU_COLLECT(mTraceGpuCtx, frameData.mCommandBuffer);

		frameData.mCommandBuffer->End();

		UpdateBindlessResources();

		// Submit command buffer
		const vk::Semaphore submitSemaphore = mSubmitSemaphores[mCurrentSwapchainImageIndex];

		const vk::CommandBufferSubmitInfo bufferSubmitInfo = frameData.mCommandBuffer->CreateSubmitInfo();
		const vk::SemaphoreSubmitInfo waitSemaphore = VkInit::SemaphoreSubmitInfo(frameData.mImageAcquiredSemaphore, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		const vk::SemaphoreSubmitInfo signalSemaphore = VkInit::SemaphoreSubmitInfo(submitSemaphore, vk::PipelineStageFlagBits2::eAllGraphics);
		const vk::SubmitInfo2 submitInfo = VkInit::SubmitInfo(bufferSubmitInfo, &signalSemaphore, &waitSemaphore);
		TRACE_ZONE(QueueSubmit, "Vulkan Queue Submit")
		CHECK_VULKAN_HPP(mVkGraphicsQueue.submit2(1, &submitInfo, frameData.mCommandBufferExecutedFence));
		TRACE_ZONE_END(QueueSubmit)

		// Present swapchain texture
		const vk::PresentInfoKHR presentInfo = VkInit::PresentInfo(mVkSwapchain, submitSemaphore, mCurrentSwapchainImageIndex);

		TRACE_ZONE(VKPresent, "Vulkan Present Frame")
		const vk::Result presentResult = mVkGraphicsQueue.presentKHR(&presentInfo);
		TRACE_ZONE_END(VKPresent)

		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
		{
			mbRequestedSwapchainResize = true;
			return false;
		}

		CHECK_VULKAN_HPP_MSG(presentResult, "Cannot present swapchain image.");
		AdvanceFrameCounters();

		return true;
	}

	void FGPUDevice::UpdateBindlessResources()
	{
		TRACE_ZONE_SCOPED()

		// todo: sort `mBindlessTexturesToUpdate` to reduce memory jumps (?)

		std::vector<vk::WriteDescriptorSet> descriptorWrites;
		descriptorWrites.reserve(mBindlessResourcesToUpdate.size() * 2);

		const FDescriptorSet* targetDescriptorSet = AccessDescriptorSet(mBindlessResourcesSet);

		std::vector<vk::DescriptorImageInfo> imageBindings;
		imageBindings.reserve(mBindlessResourcesToUpdate.size() * 2);

		vk::WriteDescriptorSet writeDescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstSet = targetDescriptorSet->mVkDescriptorSet;

		for (const FBindlessResourceUpdateRequest& request : mBindlessResourcesToUpdate)
		{
			writeDescriptorSet.dstArrayElement = request.mBindingIndex;;

			switch (request.mType)
			{
			case EResourceType::RWTexture:
				{
					const FTexture* textureToBind = AccessTexture(THandle<FTexture>(request.mHandle));
					writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageImage;
					writeDescriptorSet.dstBinding = BindlessResourcesBindings::kStorageImage;

					vk::DescriptorImageInfo& imageBinding = imageBindings.emplace_back();
					imageBinding.imageView = textureToBind->mVkImageView;
					imageBinding.imageLayout = vk::ImageLayout::eGeneral;
					imageBinding.sampler = nullptr;

					writeDescriptorSet.setImageInfo(imageBinding);

					descriptorWrites.push_back(writeDescriptorSet);

					// No brake by design. We want to bind RWTexture both as SampledImage and StorageImage
					break;
				}
			case EResourceType::Texture:
				{
					const FTexture* textureToBind = AccessTexture(THandle<FTexture>(request.mHandle));
					writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
					writeDescriptorSet.dstBinding = BindlessResourcesBindings::kSampledImage;

					vk::DescriptorImageInfo& imageBinding = imageBindings.emplace_back();
					imageBinding.imageView = textureToBind->mVkImageView;
					imageBinding.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
					imageBinding.sampler = nullptr;

					writeDescriptorSet.setImageInfo(imageBinding);

					descriptorWrites.push_back(writeDescriptorSet);

					break;
				}
			case EResourceType::Sampler:
				{
					const FSampler* samplerToBind = AccessSampler(THandle<FSampler>(request.mHandle));
					writeDescriptorSet.descriptorType = vk::DescriptorType::eSampler;
					writeDescriptorSet.dstBinding = BindlessResourcesBindings::kSampler;

					vk::DescriptorImageInfo& samplerBinding = imageBindings.emplace_back();
					samplerBinding.imageLayout = vk::ImageLayout::eUndefined;
					samplerBinding.imageView = nullptr;
					samplerBinding.sampler = samplerToBind->mVkSampler;

					writeDescriptorSet.setImageInfo(samplerBinding);

					descriptorWrites.push_back(writeDescriptorSet);

					break;
				}
			default: ;
			}
		}

		if (descriptorWrites.empty() == false)
		{
			mVkDevice.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		mBindlessResourcesToUpdate.clear();
	}

	void FGPUDevice::WaitIdle() const
	{
		TRACE_ZONE_SCOPED()

		CHECK_VULKAN_HPP(mVkDevice.waitIdle());
	}

	void FGPUDevice::ImmediateSubmit(const FOnImmediateSubmit& immediateSubmitDelegate)
	{
		if (immediateSubmitDelegate.IsBound())
		{
			TRACE_ZONE_SCOPED_N("Immediate submit")

			CHECK_VULKAN_HPP(mVkDevice.resetFences({mImmediateCommandsFence}));
			CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(mImmediateCommandsPool));

			mImmediateCommandsBuffer->Reset();
			mImmediateCommandsBuffer->Begin();
			immediateSubmitDelegate.Execute(*mImmediateCommandsBuffer);
			mImmediateCommandsBuffer->End();

			const vk::CommandBufferSubmitInfo bufferSubmitInfo = mImmediateCommandsBuffer->CreateSubmitInfo();
			const vk::SubmitInfo2 submitInfo = VkInit::SubmitInfo(bufferSubmitInfo, nullptr, nullptr);

			{
				TRACE_ZONE_SCOPED_N("Queue submit")
				CHECK_VULKAN_HPP(mVkGraphicsQueue.submit2(1, &submitInfo, mImmediateCommandsFence));
			}

			{
				TRACE_ZONE_SCOPED_N("Wait for fence")
				CHECK_VULKAN_HPP(mVkDevice.waitForFences({mImmediateCommandsFence}, vk::True, kDefaultTimeout));
			}
		}
	}

	void FGPUDevice::DestroySwapChain()
	{
		mVkDevice.destroySwapchainKHR(mVkSwapchain);

		for (uint32 imageId = 0; imageId < mNumSwapChainImages; ++imageId)
		{
			FTexture* texture = AccessTexture(mSwapChainTextures[imageId]);
			mVkDevice.destroyImageView(texture->mVkImageView);
			mTexturePool->Release(mSwapChainTextures[imageId]);

			mVkDevice.destroySemaphore(mSubmitSemaphores[imageId]);
		}

		for (uint32 imageId = 0; imageId < kMaxSwapChainImages; ++imageId)
		{
			mSwapChainTextures[imageId].Reset();
		}

		mNumSwapChainImages = 0;
	}

	void FGPUDevice::DestroyFrameDatas()
	{
		TURBO_LOG(LogGPUDevice, Info, "Destroying frames data")

		for (FBufferedFrameData& frameData : mFrameDatas)
		{
			if (frameData.mDescriptorPoolHandle)
			{
				DestroyDescriptorPool(frameData.mDescriptorPoolHandle);
			}

			if (frameData.mCommandBufferExecutedFence)
			{
				mVkDevice.destroyFence(frameData.mCommandBufferExecutedFence);
				frameData.mCommandBufferExecutedFence = nullptr;
			}

			if (frameData.mImageAcquiredSemaphore)
			{
				mVkDevice.destroySemaphore(frameData.mImageAcquiredSemaphore);
				frameData.mImageAcquiredSemaphore = nullptr;
			}

			if (frameData.mCommandPool)
			{
				CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(frameData.mCommandPool));
				mVkDevice.destroyCommandPool(frameData.mCommandPool);
			}
		}

		for (FBufferedFrameData& frameData : mFrameDatas)
		{
			frameData.mDestroyQueue.Flush(*this);
		}
	}

	void FGPUDevice::DestroyImmediateCommands()
	{
		if (mImmediateCommandsFence)
		{
			mVkDevice.destroyFence(mImmediateCommandsFence);
		}

		if (mImmediateCommandsPool)
		{
			CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(mImmediateCommandsPool));
			mVkDevice.destroyCommandPool(mImmediateCommandsPool);
			mImmediateCommandsBuffer = nullptr;
		}
	}

	void FGPUDevice::DestroyBindlessResources()
	{
		DestroyDescriptorSetLayout(mBindlessResourcesLayout);
		DestroyDescriptorPool(mBindlessResourcesPool);

		mBindlessResourcesLayout.Reset();
		mBindlessResourcesPool.Reset();
		mBindlessResourcesSet.Reset();
	}

	void FGPUDevice::AdvanceFrameCounters()
	{
		TURBO_CHECK(mNumSwapChainImages > 0)
		mBufferedFrameIndex = (mBufferedFrameIndex + 1) % kMaxBufferedFrames;

		++mRenderedFrames;
	}

	void FGPUDevice::InitVulkanTexture(const FTextureBuilder& builder, THandle<FTexture> handle, FTexture* texture, FTextureCold* textureCold)
	{
		*texture = {};

		textureCold->mFormat = builder.mFormat;

		textureCold->mWidth = builder.mWidth;
		textureCold->mHeight = builder.mHeight;
		textureCold->mDepth = builder.mDepth;
		textureCold->mNumMips = builder.mNumMips;

		textureCold->mHandle = handle;
		textureCold->mName = builder.mName;

		vk::ImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.format = builder.mFormat;
		imageCreateInfo.imageType = VkConvert::ToVkImageType(builder.mType);
		imageCreateInfo.extent.width = builder.mWidth;
		imageCreateInfo.extent.height = builder.mHeight;
		imageCreateInfo.extent.depth = builder.mDepth;
		imageCreateInfo.mipLevels = builder.mNumMips;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;

		const bool bRenderTarget = TEST_FLAG(builder.mFlags, ETextureFlags::RenderTarget);
		const bool bStorageImage = TEST_FLAG(builder.mFlags, ETextureFlags::StorageImage);

		using EImgUsage = vk::ImageUsageFlagBits;

		imageCreateInfo.usage = EImgUsage::eSampled;
		imageCreateInfo.usage |= bStorageImage ? EImgUsage::eStorage : static_cast<EImgUsage>(0);
		if (TextureFormat::HasDepthOrStencil(builder.mFormat))
		{
			imageCreateInfo.usage |= EImgUsage::eDepthStencilAttachment;
		}
		else
		{
			imageCreateInfo.usage |= EImgUsage::eTransferDst;
			imageCreateInfo.usage |= bRenderTarget ? EImgUsage::eTransferSrc : static_cast<EImgUsage>(0);
			imageCreateInfo.usage |= bRenderTarget ? EImgUsage::eColorAttachment : static_cast<EImgUsage>(0);
		}

		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

		vma::AllocationCreateInfo imageAllocationInfo = {};
		imageAllocationInfo.usage = vma::MemoryUsage::eAutoPreferDevice;
		imageAllocationInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		{
			std::pair<vk::Image, vma::Allocation> allocationResult;
			CHECK_VULKAN_RESULT(allocationResult, mVmaAllocator.createImage(imageCreateInfo, imageAllocationInfo))
			std::tie(texture->mVkImage, texture->mImageAllocation) = allocationResult;
		}

		SetResourceName(texture->mVkImage, textureCold->mName);

		vk::ImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.image = texture->mVkImage;
		viewCreateInfo.viewType = VkConvert::ToVkImageViewType(builder.mType);
		viewCreateInfo.format = builder.mFormat;
		viewCreateInfo.subresourceRange.aspectMask =
			TextureFormat::HasDepthOrStencil(builder.mFormat) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
		viewCreateInfo.subresourceRange.levelCount = builder.mNumMips;
		viewCreateInfo.subresourceRange.layerCount = 1;


		CHECK_VULKAN_RESULT(texture->mVkImageView, mVkDevice.createImageView(viewCreateInfo));

		SetResourceName(texture->mVkImageView, builder.mName);
	}

	void FGPUDevice::UploadTextureUsingStagingBuffer(THandle<FTexture> handle, std::span<const byte> data)
	{
		// Create staging buffer
		const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(data.size());

		const THandle<FBuffer> stagingBuffer = CreateBuffer(stagingBufferBuilder);
		void* stagingMappedAddress = AccessBuffer(stagingBuffer)->GetMappedAddress();
		std::memcpy(stagingMappedAddress, data.data(), data.size_bytes());

		// copy buffer to image
		ImmediateSubmit(FOnImmediateSubmit::CreateLambda(
				[&](FCommandBuffer& cmd)
				{
					cmd.BufferBarrier(
						stagingBuffer,
						vk::AccessFlagBits2::eHostWrite,
						vk::PipelineStageFlagBits2::eHost,
						vk::AccessFlagBits2::eTransferRead,
						vk::PipelineStageFlagBits2::eTransfer
						);
					cmd.TransitionImage(handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
					cmd.CopyBufferToTexture(stagingBuffer, handle, 0, 0);
					cmd.TransitionImage(handle, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
				})
		);

		DestroyBuffer(stagingBuffer);
	}

	void FGPUDevice::DestroyBufferImmediate(const FBufferDestroyer& destroyer)
	{
		mVmaAllocator.destroyBuffer(destroyer.mVkBuffer, destroyer.mAllocation);
		mBufferPool->Release(destroyer.mHandle);
	}

	void FGPUDevice::DestroyTextureImmediate(const FTextureDestroyer& destroyer)
	{
		mVmaAllocator.destroyImage(destroyer.mImage, destroyer.mImageAllocation);
		mVkDevice.destroyImageView(destroyer.mImageView);
		mTexturePool->Release(destroyer.mHandle);
	}

	void FGPUDevice::DestroySamplerImmediate(const FSamplerDestroyer& destroyer)
	{
		mVkDevice.destroySampler(destroyer.mVkSampler);
		mSamplerPool->Release(destroyer.mHandle);
	}

	void FGPUDevice::DestroyPipelineImmediate(const FPipelineDestroyer& destroyer)
	{
		mVkDevice.destroyPipelineLayout(destroyer.mLayout);
		mVkDevice.destroyPipeline(destroyer.mPipeline);
		mPipelinePool->Release(destroyer.mHandle);
	}

	void FGPUDevice::DestroyDescriptorPoolImmediate(const FDescriptorPoolDestroyer& destroyer)
	{
		ResetDescriptorPool(destroyer.mhandle);
		mVkDevice.destroyDescriptorPool(destroyer.mVkDescriptorPool);
		mDescriptorPoolPool->Release(destroyer.mhandle);
	}

	void FGPUDevice::DestroyDescriptorSetLayoutImmediate(const FDescriptorSetLayoutDestroyer& destroyer)
	{
		mVkDevice.destroyDescriptorSetLayout(destroyer.mVkLayout);
		mDescriptorSetLayoutPool->Release(destroyer.mHandle);
	}

	void FGPUDevice::DestroyShaderStateImmediate(const FShaderStateDestroyer& destroyer)
	{
		for (uint32 shaderId = 0; shaderId < destroyer.mNumActiveShaders; ++shaderId)
		{
			mVkDevice.destroyShaderModule(destroyer.mModules[shaderId]);
		}

		mShaderStatePool->Release(destroyer.mHandle);
	}

	VkBool32 FGPUDevice::ValidationLayerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData
		)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			TURBO_LOG(LogGPUDevice, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			// Messages with info severity are very verbose, so I reduced its verbosity to display.
			TURBO_LOG(LogGPUDevice, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			TURBO_LOG(LogGPUDevice, Warn, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			TURBO_LOG(LogGPUDevice, Error, "{}", callbackData->pMessage)
			TURBO_DEBUG_BREAK();
		}

		return VK_FALSE;
	}

} // Turbo
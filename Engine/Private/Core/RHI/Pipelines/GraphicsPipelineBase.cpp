#include "Core/RHI/Pipelines/GraphicsPipelineBase.h"

#include "Core/CoreUtils.h"
#include "Core/RHI/Utils/VulkanUtils.h"

namespace Turbo {

	void FGraphicsPipelineBase::Init()
	{
		vk::GraphicsPipelineCreateInfo pipelineCI{};
		const std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = InitStages();
		pipelineCI.setStages(pipelineStages);
		const vk::PipelineVertexInputStateCreateInfo vertexInputState = InitVertexInput();
		pipelineCI.setPVertexInputState(&vertexInputState);
		const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = InitInputAssembly();
		pipelineCI.setPInputAssemblyState(&inputAssemblyState);
		const vk::PipelineViewportStateCreateInfo viewportState = InitViewportState();
		pipelineCI.setPViewportState(&viewportState);
		const vk::PipelineRasterizationStateCreateInfo rasterizationState = InitRasterizationState();
		pipelineCI.setPRasterizationState(&rasterizationState);
		const vk::PipelineMultisampleStateCreateInfo multisampleState = InitMultisampleState();
		pipelineCI.setPMultisampleState(&multisampleState);
		const std::vector<vk::PipelineColorBlendAttachmentState> colorAttachments = GetColorAttachments();
		const vk::PipelineColorBlendStateCreateInfo colorBlendState = InitColorBlending(colorAttachments);
		pipelineCI.setPColorBlendState(&colorBlendState);
		const vk::PipelineDepthStencilStateCreateInfo depthStencilState = InitDepthStencilState();
		pipelineCI.setPDepthStencilState(&depthStencilState);

		vk::PipelineDynamicStateCreateInfo dynamicStateCI{};
		const std::vector<vk::DynamicState> dynamicStates = GetDynamicStates();
		dynamicStateCI.setDynamicStates(dynamicStates);
		pipelineCI.setPDynamicState(&dynamicStateCI);

		const std::vector<vk::Format> attachmentFormats = GetColorAttachmentFormats();
		const vk::PipelineRenderingCreateInfo pipelineRendering = InitRenderingState(attachmentFormats);
		pipelineCI.setPNext(&pipelineRendering);

		std::vector<vk::PushConstantRange> pushConstantRanges = InitPushConstantRanges();
		std::vector<vk::DescriptorSetLayout> descriptorSetsLayouts = InitDescriptorSetLayouts();

		vk::PipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.setPushConstantRanges(pushConstantRanges);
		layoutCreateInfo.setSetLayouts(descriptorSetsLayouts);

		CHECK_VULKAN_RESULT(mPipelineLayout, mDevice->Get().createPipelineLayout(layoutCreateInfo));
		pipelineCI.setLayout(mPipelineLayout);

		CHECK_VULKAN_RESULT(mPipeline, mDevice->Get().createGraphicsPipeline(nullptr, pipelineCI));

		DestroyShaderModules(pipelineStages);
	}

	void FGraphicsPipelineBase::DestroyShaderModules(const std::vector<vk::PipelineShaderStageCreateInfo>& pipelineStages) const
	{
		std::set<vk::ShaderModule> modules;
		for (const vk::PipelineShaderStageCreateInfo& stage : pipelineStages)
		{
			modules.insert(stage.module);
		}

		for (const vk::ShaderModule& module : modules)
		{
			mDevice->Get().destroyShaderModule(module);
		}
	}

	void FGraphicsPipelineBase::Destroy()
	{
		// Move to destroy queue
		mDevice->Get().destroyPipelineLayout(mPipelineLayout);
		mDevice->Get().destroyPipeline(mPipeline);

	}

	void FGraphicsPipelineBase::Bind(const vk::CommandBuffer& cmd)
	{
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
	}

	std::vector<vk::PipelineShaderStageCreateInfo> FGraphicsPipelineBase::InitStages()
	{
		vk::ShaderModule vertexShaderModule = nullptr;
		vk::ShaderModule fragmentShaderModule = nullptr;

		// todo: Load shader binary async or preload all shader binaries
		std::vector<uint32> shaderData = FCoreUtils::ReadWholeFile<uint32>(mVertexShaderPath);
		TURBO_CHECK_MSG(shaderData.size() > 0, "Error creating shader module. \"{}\".", mVertexShaderPath);
		vertexShaderModule = fragmentShaderModule = VulkanUtils::CreateShaderModule(mDevice, shaderData);
		TURBO_CHECK_MSG(vertexShaderModule, "Error creating shader module. \"{}\".", mVertexShaderPath);

		if (mVertexShaderPath != mFragmentShaderPath)
		{
			shaderData = FCoreUtils::ReadWholeFile<uint32>(mFragmentShaderPath);
			TURBO_CHECK_MSG(shaderData.size() > 0, "Error creating shader module. \"{}\".", mVertexShaderPath);
			fragmentShaderModule = VulkanUtils::CreateShaderModule(mDevice, shaderData);
			TURBO_CHECK_MSG(vertexShaderModule, "Error creating shader module. \"{}\".", mVertexShaderPath);
		}

		std::vector<vk::PipelineShaderStageCreateInfo> result;

		auto addShaderStage = [&](vk::ShaderStageFlagBits stage, const vk::ShaderModule& module)
		{
			result.emplace_back();
			vk::PipelineShaderStageCreateInfo& vertexStage = *result.rbegin();
			vertexStage.setStage(stage);
			vertexStage.setModule(module);
			vertexStage.setPName(GetShaderName(stage));
		};

		addShaderStage(vk::ShaderStageFlagBits::eVertex, vertexShaderModule);
		addShaderStage(vk::ShaderStageFlagBits::eFragment, fragmentShaderModule);

		return result;
	}

	const char* FGraphicsPipelineBase::GetShaderName(vk::ShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return "vsMain";
		case vk::ShaderStageFlagBits::eFragment:
			return "psMain";
		default:
			TURBO_UNINPLEMENTED();
			return "";
		}
	}

	vk::PipelineColorBlendStateCreateInfo FGraphicsPipelineBase::InitColorBlending(const std::vector<vk::PipelineColorBlendAttachmentState>& colorBlendAttachments)
	{
		// No blending by default
		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.setLogicOpEnable(false);
		colorBlending.setLogicOp(vk::LogicOp::eCopy);
		colorBlending.setAttachments(colorBlendAttachments);

		return colorBlending;
	}

	std::vector<vk::PipelineColorBlendAttachmentState> FGraphicsPipelineBase::GetColorAttachments()
	{
		std::vector<vk::PipelineColorBlendAttachmentState> result;

		result.emplace_back();
		vk::PipelineColorBlendAttachmentState& attachment = *result.rbegin();
		attachment.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA;
		attachment.blendEnable = false;

		return result;
	}

	vk::PipelineRenderingCreateInfo FGraphicsPipelineBase::InitRenderingState(const std::vector<vk::Format>& attachmentFormats)
	{
		vk::PipelineRenderingCreateInfo renderingState;
		renderingState.setColorAttachmentFormats(attachmentFormats);
		renderingState.setDepthAttachmentFormat(GetDepthFormat());
		renderingState.setStencilAttachmentFormat(GetStencilFormat());

		return renderingState;
	}

	std::vector<vk::Format> FGraphicsPipelineBase::GetColorAttachmentFormats()
	{
		// default format of draw image
		return {vk::Format::eR16G16B16A16Sfloat};
	}

	vk::Format FGraphicsPipelineBase::GetDepthFormat()
	{
		return vk::Format::eUndefined;
	}

	vk::Format FGraphicsPipelineBase::GetStencilFormat()
	{
		return vk::Format::eUndefined;
	}

	vk::PipelineDepthStencilStateCreateInfo FGraphicsPipelineBase::InitDepthStencilState()
	{
		// TODO: Enable depth test by default
		vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.setDepthTestEnable(false);
		depthStencilState.setDepthWriteEnable(false);
		depthStencilState.setDepthCompareOp(vk::CompareOp::eNever);
		depthStencilState.setDepthBoundsTestEnable(false);
		depthStencilState.setFront({});
		depthStencilState.setBack({});
		depthStencilState.minDepthBounds = 0.f;
		depthStencilState.maxDepthBounds = 1.f;

		return depthStencilState;
	}

	vk::PipelineViewportStateCreateInfo FGraphicsPipelineBase::InitViewportState()
	{
		// Single viewport by default
		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.setViewportCount(1);
		viewportState.setScissorCount(1);

		return viewportState;
	}

	vk::PipelineVertexInputStateCreateInfo FGraphicsPipelineBase::InitVertexInput()
	{
		// No input by default
		return vk::PipelineVertexInputStateCreateInfo{};
	}

	std::vector<vk::DynamicState> FGraphicsPipelineBase::GetDynamicStates()
	{
		return {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	}

	vk::PipelineInputAssemblyStateCreateInfo FGraphicsPipelineBase::InitInputAssembly()
	{
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.setTopology(GetTopology());
		inputAssemblyState.setPrimitiveRestartEnable(false);

		return inputAssemblyState;
	}

	vk::PrimitiveTopology FGraphicsPipelineBase::GetTopology()
	{
		return vk::PrimitiveTopology::eTriangleList;
	}

	vk::PipelineRasterizationStateCreateInfo FGraphicsPipelineBase::InitRasterizationState()
	{
		vk::PipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.setPolygonMode(GetPolygonMode());
		rasterizationState.setLineWidth(GetPolygonLineWidth());

		return rasterizationState;
	}

	vk::PolygonMode FGraphicsPipelineBase::GetPolygonMode()
	{
		return vk::PolygonMode::eFill;
	}

	float FGraphicsPipelineBase::GetPolygonLineWidth()
	{
		return 1.f;
	}

	vk::CullModeFlags FGraphicsPipelineBase::GetCullMode()
	{
		return vk::CullModeFlagBits::eBack;
	}

	vk::FrontFace FGraphicsPipelineBase::GetFrontFace()
	{
		return vk::FrontFace::eClockwise;
	}

	vk::PipelineMultisampleStateCreateInfo FGraphicsPipelineBase::InitMultisampleState()
	{
		// MSAA off by default
		vk::PipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.setSampleShadingEnable(false);
		multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		multisampleState.setMinSampleShading(0.f);
		multisampleState.setPSampleMask(nullptr);
		multisampleState.setAlphaToCoverageEnable(false);
		multisampleState.setAlphaToOneEnable(false);

		return multisampleState;
	}

	std::vector<vk::PushConstantRange> FGraphicsPipelineBase::InitPushConstantRanges()
	{
		return {};
	}

	std::vector<vk::DescriptorSetLayout> FGraphicsPipelineBase::InitDescriptorSetLayouts()
	{
		return {};
	}
} // Turbo
#pragma once

#include "CommonMacros.h"
#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	class FGraphicsPipelineBase
	{
		GENERATED_BODY(FGraphicsPipelineBase)

	public:
		explicit FGraphicsPipelineBase() = delete;
		virtual ~FGraphicsPipelineBase() = default;

	protected:

		explicit FGraphicsPipelineBase(FVulkanDevice* device, const std::string_view shaderPath)
			: FGraphicsPipelineBase(device, shaderPath, shaderPath)
		{}

		explicit FGraphicsPipelineBase(FVulkanDevice* device, const std::string_view vertexPath, const std::string_view fragmentPath)
			: mDevice(device)
			, mVertexShaderPath(vertexPath)
			, mFragmentShaderPath(fragmentPath)
		{}

	public:
		void Init();
		void Destroy();

		virtual void Bind(const vk::CommandBuffer& cmd);

		template<typename PushConstantType>
		void PushConstants(const vk::CommandBuffer& cmd, vk::ShaderStageFlags shaderStage, const PushConstantType& data);

	public:
		[[nodiscard]] vk::Pipeline GetPipeline() const { return mPipeline; }
		[[nodiscard]] vk::PipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	protected:
		virtual std::vector<vk::PipelineShaderStageCreateInfo> InitStages();
		virtual const char* GetShaderName(vk::ShaderStageFlagBits stage);

		virtual vk::PipelineColorBlendStateCreateInfo InitColorBlending(const std::vector<vk::PipelineColorBlendAttachmentState>& colorBlendAttachments);
		virtual std::vector<vk::PipelineColorBlendAttachmentState> GetColorAttachments();

		virtual vk::PipelineRenderingCreateInfo InitRenderingState(const std::vector<vk::Format>& attachmentFormats);
		virtual std::vector<vk::Format> GetColorAttachmentFormats();
		virtual vk::Format GetDepthFormat();
		virtual vk::Format GetStencilFormat();

		virtual vk::PipelineDepthStencilStateCreateInfo InitDepthStencilState();

		virtual vk::PipelineViewportStateCreateInfo InitViewportState();

		virtual vk::PipelineVertexInputStateCreateInfo InitVertexInput();

		virtual std::vector<vk::DynamicState> GetDynamicStates();

		// Input Assembly
		virtual vk::PipelineInputAssemblyStateCreateInfo InitInputAssembly();
		virtual vk::PrimitiveTopology GetTopology();

		// Rasterizer
		virtual vk::PipelineRasterizationStateCreateInfo InitRasterizationState();
		virtual vk::PolygonMode GetPolygonMode();
		virtual float GetPolygonLineWidth();
		virtual vk::CullModeFlags GetCullMode();
		virtual vk::FrontFace GetFrontFace();

		virtual vk::PipelineMultisampleStateCreateInfo InitMultisampleState();

		virtual std::vector<vk::PushConstantRange> InitPushConstantRanges();
		virtual std::vector<vk::DescriptorSetLayout> InitDescriptorSetLayouts();

	private:
		void DestroyShaderModules(const std::vector<vk::PipelineShaderStageCreateInfo>& pipelineStages) const;

	private:
		FVulkanDevice* mDevice = nullptr;

		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mPipelineLayout = nullptr;

	private:
		const std::string mVertexShaderPath{};
		const std::string mFragmentShaderPath{};
	};

	template <typename PushConstantType>
	void FGraphicsPipelineBase::PushConstants(const vk::CommandBuffer& cmd, vk::ShaderStageFlags shaderStage, const PushConstantType& data)
	{
		cmd.pushConstants(mPipelineLayout, shaderStage, 0, sizeof(data), &data);
	}

} // Turbo

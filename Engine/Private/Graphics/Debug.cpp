#include "Graphics/Debug.h"

#include "Debug/RenderDocFrameDebuggerAPI.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/FrameGraph/RenderGraph.h"

namespace Turbo
{
#if WITH_DEBUG_RENDERING_FEATURES
	FScopedLabelRegion::FScopedLabelRegion(FCommandBuffer& commandBuffer, std::string_view label, glm::float4 color)
		: mCommandBuffer(&commandBuffer)
	{
		TURBO_CHECK(mCommandBuffer)
		commandBuffer.BeginDebugUtilsLabel(label, color);
	}

	FScopedLabelRegion::~FScopedLabelRegion()
	{
		TURBO_CHECK(mCommandBuffer)
		mCommandBuffer->EndDebugUtilsLabel();
	}
#endif // WITH_DEBUG_RENDERING_FEATURES

	IFrameDebuggerAPI* IFrameDebuggerAPI::Get()
	{
		static IFrameDebuggerAPI* instance = nullptr;
		if (instance == nullptr)
		{
			FRenderDocFrameDebuggerAPI* renderDocFrameDebuggerAPI = new FRenderDocFrameDebuggerAPI();
			if (renderDocFrameDebuggerAPI->Init())
			{
				instance = renderDocFrameDebuggerAPI;
			}
			else
			{
				instance = new FNullFrameDebuggerAPI();
			}
		}

		return instance;
	}

	FScopedRenderCapture::FScopedRenderCapture(bool bCapture, FRenderGraphBuilder& graphBuilder)
	{
		if (bCapture)
		{
			mGraphBuilder = &graphBuilder;

			static FName passName = FName("BeginRenderCapture");
			graphBuilder.AddPass(
				passName,
				FRGSetupPassDelegate::CreateLambda([&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Compute;
				}),
				FRGExecutePassDelegate::CreateLambda([](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					IFrameDebuggerAPI::Get()->BeginCapture(&gpu, nullptr);
				})
			);
		}
	}

	FScopedRenderCapture::~FScopedRenderCapture()
	{
		if (mGraphBuilder)
		{
			static FName passName = FName("EndRenderCapture");
			mGraphBuilder->AddPass(
				passName,
				FRGSetupPassDelegate::CreateLambda([&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Compute;
				}),
				FRGExecutePassDelegate::CreateLambda([](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					IFrameDebuggerAPI::Get()->EndCapture(&gpu, nullptr);
				})
			);
		}
	}
} // Turbo

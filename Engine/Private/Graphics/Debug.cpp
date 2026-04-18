#include "Graphics/Debug.h"

#include "Debug/RenderDocFrameDebuggerAPI.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/GPUDevice.h"
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

	void IFrameDebuggerAPI::Emplace()
	{
		IFrameDebuggerAPI& renderDoc = entt::locator<IFrameDebuggerAPI>::emplace<FRenderDocFrameDebuggerAPI>();
		if (renderDoc.Init() == false)
		{
			entt::locator<IFrameDebuggerAPI>::emplace<FNullFrameDebuggerAPI>();
		}
	}
#endif // WITH_DEBUG_RENDERING_FEATURES

	FScopedRenderCapture::FScopedRenderCapture(bool bCapture, FRenderGraphBuilder& graphBuilder)
	{
		if (bCapture)
		{
			mGraphBuilder = &graphBuilder;

			static FName passName = FName("BeginRenderCapture");
			auto pass = graphBuilder.AddPass(passName, EPassType::Compute);
			pass->mExecutePass.BindLambda(
				[](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					entt::locator<IFrameDebuggerAPI>::value().BeginCapture(&gpu, nullptr);
				}
			);
		}
	}

	FScopedRenderCapture::~FScopedRenderCapture()
	{
		if (mGraphBuilder)
		{
			static FName passName = FName("EndRenderCapture");
			FRGPassInitializer pass = mGraphBuilder->AddPass(passName, EPassType::Compute);
			pass->mExecutePass.BindLambda(
				[](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					entt::locator<IFrameDebuggerAPI>::value().EndCapture(&gpu, nullptr);
				}
			);
		}
	}
} // Turbo

#include "Graphics/Debug.h"

#include "Core/Utils/StringUtils.h"
#include "Debug/RenderDocFrameDebuggerAPI.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraph.h"

namespace Turbo
{
#if WITH_DEBUG_RENDERING_FEATURES
	FScopedLabelRegion::FScopedLabelRegion(FCommandBuffer& commandBuffer, FName label, glm::float4 color)
		: mCommandBuffer(&commandBuffer)
	{
		TURBO_CHECK(mCommandBuffer && mCommandBuffer->GetGPUDevice())
		commandBuffer.BeginDebugUtilsLabel(label.ToCString(), color);

#if WITH_PROFILER
		mGPUZone = new tracy::VkCtxScope(
			commandBuffer.GetGPUDevice()->GetTraceGpuCtx(),
			0,
			StringUtils::kEmptyCString, 0,
			StringUtils::kEmptyCString, 0,
			label.ToCString(), std::strlen(label.ToCString()),
			commandBuffer.GetVkCommandBuffer(),
			true
		);

		mCPUZone = new tracy::ScopedZone(
			0,
			StringUtils::kEmptyCString, 0,
			StringUtils::kEmptyCString, 0,
			label.ToCString(), std::strlen(label.ToCString()),
			TRACY_CALLSTACK,
			true
			);
#endif // WITH_PROFILER
	}

	FScopedLabelRegion::~FScopedLabelRegion()
	{
		TURBO_CHECK(mCommandBuffer)
		mCommandBuffer->EndDebugUtilsLabel();

		delete mGPUZone;
		delete mCPUZone;
	}

#endif // WITH_DEBUG_RENDERING_FEATURES

	void IFrameDebuggerAPI::Emplace()
	{
		IFrameDebuggerAPI& renderDoc = entt::locator<IFrameDebuggerAPI>::emplace<FRenderDocFrameDebuggerAPI>();
		if (renderDoc.Init() == false)
		{
			entt::locator<IFrameDebuggerAPI>::emplace<FNullFrameDebuggerAPI>();
		}
	}

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

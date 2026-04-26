#include "Rendering/GameViewportLayer.h"

#include "Core/Window.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	class FGeometryBuffer;

	void FGameViewportLayer::Start()
	{
		const glm::uint2 framebufferSize = entt::locator<FWindow>::value().GetFrameBufferSize();
		entt::locator<FGPUDevice>::value().SetMainViewportSize(framebufferSize);
	}

	void FGameViewportLayer::Shutdown()
	{
	}

	void FGameViewportLayer::BeginPresentingFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentImage)
	{
		FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
		geometryBuffer.BlitToPresent(graphBuilder, presentImage);
	}

	bool FGameViewportLayer::ShouldRender()
	{
		return true;
	}

	void FGameViewportLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FResizeWindowEvent>(event, this, &FGameViewportLayer::HandleResizeWindowEvent);
	}

	template<>
	FName GetStaticLayerName<FGameViewportLayer>()
	{
		static const FName kLayerName("GameViewport");
		return kLayerName;
	}

	FName FGameViewportLayer::GetName()
	{
		return GetStaticLayerName<FGameViewportLayer>();
	}

	void FGameViewportLayer::HandleResizeWindowEvent(FResizeWindowEvent& event)
	{
		entt::locator<FGPUDevice>::value().SetMainViewportSize(event.mNewWindowSize);
	}
} // Turbo
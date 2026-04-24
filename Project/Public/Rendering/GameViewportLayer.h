#pragma once
#include "Core/WindowEvents.h"
#include "Layers/Layer.h"

namespace Turbo
{
	class FGameViewportLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void BeginPresentingFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentImage) override;
		virtual bool ShouldRender() override;
		virtual void OnEvent(FEventBase& event) override;

		virtual FName GetName() override;

	private:
		void HandleResizeWindowEvent(FResizeWindowEvent& event);
	};
} // Turbo

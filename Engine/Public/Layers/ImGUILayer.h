#pragma once

#include "Layer.h"
#include "Graphics/Resources.h"
#include "SDL3/SDL_events.h"

namespace Turbo
{
	class FImGuiLayer final : public ILayer
	{
		GENERATED_BODY(FImGuiLayer, ILayer)

	public:

		/** Service Api */
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void BeginTick(double deltaTime) override;
		virtual void EndTick(double deltaTime) override;
		virtual void BeginPresentingFrame(FGPUDevice* gpu, FCommandBuffer* cmd, THandle<FTexture> PresentImage) override;

		virtual bool ShouldTick() override { return true; }
		virtual bool ShouldRender() override { return true; }
		/** Service Api end */

	private:
		void OnSDLEvent(SDL_Event* sdlEvent);

	public:
		virtual FName GetName() override;
	};
} // Turbo
#pragma once

#include "IService.h"
#include "Graphics/Resources.h"
#include "SDL3/SDL_events.h"

namespace Turbo
{
	class FImGuiService final : public IService
	{
		GENERATED_BODY(FImGuiService, IService)

	public:

		/** Service Api */
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void BeginTick_GameThread(float deltaTime) override;
		virtual void EndTick_GameThread(float deltaTime) override;
		virtual void BeginPresentingFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) override;
		/** Service Api end */

	private:
		void OnSDLEvent(SDL_Event* sdlEvent);

	public:
		virtual FName GetName() override;
		virtual EEngineStage GetStage() override;
	};
} // Turbo
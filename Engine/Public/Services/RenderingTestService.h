#pragma once

#include "CommonMacros.h"
#include "IService.h"

namespace Turbo
{
	class FRenderingTestService final : public IService
	{
		GENERATED_BODY(FRenderingTestService, IService);

		/** IService Interface */
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void RenderFrame_RenderThread(FGPUDevice* device, FCommandBuffer* cmd) override;

		virtual FName GetName() override;

		/** IService Interface end */

	private:
	};
} // Turbo
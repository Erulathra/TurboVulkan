#pragma once

#include "CommonMacros.h"
#include "Graphics/Resources.h"
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

		virtual void PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) override;

		virtual FName GetName() override;

		/** IService Interface end */

	private:
		FPipelineHandle mPipeline;
		FDescriptorSetLayoutHandle mSetLayout;
	};
} // Turbo
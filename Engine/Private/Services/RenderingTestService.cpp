#include "Services/RenderingTestService.h"

namespace Turbo
{
	void FRenderingTestService::Start()
	{
		TURBO_LOG(LOG_TEMP, Info, "Service start")
	}

	void FRenderingTestService::Shutdown()
	{
		TURBO_LOG(LOG_TEMP, Info, "Service shutdown")
	}

	void FRenderingTestService::RenderFrame_RenderThread(FGPUDevice* device, FCommandBuffer* cmd)
	{
		TURBO_LOG(LOG_TEMP, Info, "Render")
	}

	FName FRenderingTestService::GetName()
	{
		return mClassName;
	}

	REGISTER_SERVICE(FRenderingTestService)
} // Turbo
#include "RenderDocFrameDebuggerAPI.h"

#include "Graphics/GPUDevice.h"

namespace Turbo
{
	FRenderDocFrameDebuggerAPI::~FRenderDocFrameDebuggerAPI()
	{
	}

	bool FRenderDocFrameDebuggerAPI::Init()
	{
		uint32 result = 0;

#if PLATFORM_WINDOWS
		if(HMODULE moduleHandle = GetModuleHandleA("renderdoc.dll"))
		{
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(moduleHandle, "RENDERDOC_GetAPI");
			result = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&mApi);
		}
#else
		if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
		{
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
			result = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&mApi);
		}
#endif

		if (result == 1)
		{
			TURBO_LOG(LogRenderDoc, Info, "RenderDoc API initialized successfully");
			mApi->SetCaptureKeys(nullptr, 0);
			mApi->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);
		}
		else
		{
			TURBO_LOG(LogRenderDoc, Warn, "RenderDoc API load error, falling to NULL implementation");
		}

		mbInitialized = result == 1;
		return mbInitialized;
	}

	void FRenderDocFrameDebuggerAPI::Shutdown()
	{
		TURBO_LOG(LogRenderDoc, Info, "RenderDoc API destroyed");
		mApi->RemoveHooks();
		mbInitialized = false;
	}

	bool FRenderDocFrameDebuggerAPI::CanCapture()
	{
		return mbInitialized;
	}

	void FRenderDocFrameDebuggerAPI::BeginCapture(const FGPUDevice* gpu, const FWindow* window)
	{
		TURBO_CHECK(mbInitialized)
		TURBO_CHECK(mApi->IsFrameCapturing() == false)

		TURBO_LOG(LogRenderDoc, Info, "RenderDoc API capture started");

		mApi->StartFrameCapture(
			RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(static_cast<VkInstance>(gpu->GetVkInstance())),
			nullptr
			);
	}

	void FRenderDocFrameDebuggerAPI::EndCapture(const FGPUDevice* gpu, const FWindow* window)
	{
		TURBO_CHECK(mbInitialized)
		TURBO_CHECK(mApi->IsFrameCapturing())

		TURBO_LOG(LogRenderDoc, Info, "RenderDoc API capture ended");

		mApi->EndFrameCapture(
			RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(static_cast<VkInstance>(gpu->GetVkInstance())),
			nullptr
			);
	}

	void FRenderDocFrameDebuggerAPI::CaptureFrame()
	{
		TURBO_CHECK(mbInitialized)
		TURBO_LOG(LogRenderDoc, Info, "RenderDoc API capture frame");

		mApi->TriggerCapture();
	}
} // Turbo
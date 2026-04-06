#pragma once
#include "Graphics/Debug.h"
#include "renderdoc_app.h"

DECLARE_LOG_CATEGORY(LogRenderDoc, Display, Display)

namespace Turbo
{
	class FWindow;
	class FGPUDevice;

	class FRenderDocFrameDebuggerAPI final : public IFrameDebuggerAPI
	{
	public:
		FRenderDocFrameDebuggerAPI() = default;
		virtual ~FRenderDocFrameDebuggerAPI() override;

		virtual bool Init() override;
		virtual void Shutdown() override;
		virtual bool CanCapture() override;
		virtual void BeginCapture(const FGPUDevice* gpu, const FWindow* window) override;
		virtual void EndCapture(const FGPUDevice* gpu, const FWindow* window) override;
		virtual void CaptureFrame() override;

	private:
		RENDERDOC_API_1_4_1* mApi = nullptr;
		bool mbInitialized = false;
	};
} // Turbo

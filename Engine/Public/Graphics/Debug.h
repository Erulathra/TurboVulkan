#pragma once

namespace Turbo
{
	struct FRenderGraphBuilder;
	class FWindow;
	class FGPUDevice;
	class FCommandBuffer;

#if WITH_DEBUG_RENDERING_FEATURES

	class FScopedLabelRegion final
	{
	public:
		FScopedLabelRegion(FCommandBuffer& commandBuffer, std::string_view label, glm::float4 color = glm::float4(1.f));
		~FScopedLabelRegion();

		DELETE_COPY(FScopedLabelRegion)
	private:
		FCommandBuffer* mCommandBuffer;
	};

#define DEBUG_LABEL_REGION(commandBuffer, label) FScopedLabelRegion __label_region__(commandBuffer, label);
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) FScopedLabelRegion __label_region__(commandBuffer, label, color);
#else // WITH_DEBUG_RENDERING_FEATURES
#define DEBUG_LABEL_REGION(commandBuffer, label) {}
#define DEBUG_LABEL_REGION_COLOR(commandBuffer, label, color) {}
#endif // else WITH_DEBUG_RENDERING_FEATURES

	class IFrameDebuggerAPI
	{
	public:
		static IFrameDebuggerAPI* Get();

		virtual ~IFrameDebuggerAPI() = default;
	public:
		virtual bool Init() = 0;
		virtual void Shutdown() = 0;

		virtual bool CanCapture() { return false; }

		virtual void BeginCapture(FGPUDevice* gpu, FWindow* window) = 0;
		virtual void EndCapture(FGPUDevice* gpu, FWindow* window) = 0;

		virtual void CaptureFrame() = 0;
	};

	class FNullFrameDebuggerAPI final : public IFrameDebuggerAPI
	{
	public:
		virtual bool Init() override { return true; }
		virtual void Shutdown() override {}
		virtual void BeginCapture(FGPUDevice* gpu, FWindow* window) override {}
		virtual void EndCapture(FGPUDevice* gpu, FWindow* window) override {}
		virtual void CaptureFrame() override {}
	};

	class FScopedRenderCapture final
	{
	public:
		FScopedRenderCapture() = delete;
		FScopedRenderCapture(bool bCapture, FRenderGraphBuilder& graphBuilder);
		~FScopedRenderCapture();

		DELETE_COPY(FScopedRenderCapture)

	private:
		FRenderGraphBuilder* mGraphBuilder = nullptr;
	};
} // Turbo

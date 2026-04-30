#include "EditorLayer.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "EditorViewPort/EditorFreeCamera.h"
#include "Graphics/Debug.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraphUtils.h"
#include "Layers/ImGUILayer.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	const FName kExitName = FName("Exit");
	const FName kToggleFullscreenName = FName("ToggleFullscreen");
	const FName kFrameCapture = FName("FrameCapture");

	template<>
	FName GetStaticLayerName<FEditorLayer>()
	{
		static FName layerName = FName("EditorLayer");
		return layerName;
	}

	void FEditorLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding({kExitName, EKeys::Escape});
		inputSystem.RegisterBinding({kToggleFullscreenName, EKeys::F11});
		inputSystem.RegisterBinding({kFrameCapture, EKeys::F12});

		FEditorFreeCameraUtils::Init();
	}

	void FEditorLayer::Shutdown()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		for (THandle<FTexture> texture : mRenderedTextures)
		{
			gpu.DestroyTexture(texture);
		}
	}

	void FEditorLayer::BeginTick(double deltaTime)
	{
		FEditorFreeCameraUtils::Tick(deltaTime);
	}

	void FEditorLayer::EndTick(double deltaTime)
	{
		DrawEditorViewport();
	}

	bool FEditorLayer::ShouldTick()
	{
		return true;
	}

	void FEditorLayer::EndFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture)
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const uint32 bufferedFrameId = gpu.GetBufferedFrameId();

		if (bufferedFrameId < mRenderedTextures.size())
		{
			const FRGResourceHandle viewportTexture = graphBuilder.RegisterExternalTexture(mRenderedTextures[bufferedFrameId], ETextureLayout::Undefined);

			const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
			RenderGraphUtils::AddBlitTexturePass(graphBuilder, geometryBuffer.mColor, viewportTexture);
		}

		RenderGraphUtils::AddClearTexturePass(graphBuilder, presentTexture, ELinearColor::kBlack);
	}

	bool FEditorLayer::ShouldRender()
	{
		return true;
	}

	void FEditorLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(event, this, &FEditorLayer::HandleInputActionEvent);
		FEventDispatcher::Dispatch<FCloseWindowEvent>(event, this, &FEditorLayer::HandleCloseEvent);

		FEditorFreeCameraUtils::HandleEvent(event);
	}

	FName FEditorLayer::GetName()
	{
		return GetStaticLayerName<FEditorLayer>();
	}

	void FEditorLayer::DrawEditorViewport()
	{
		ImGui::Begin("Viewport");

		// const glm::uint2 newContentSize = currentWindow->SizeFull
		const glm::uint2 newContentSize = ImGui::GetContentRegionAvail();
		if (newContentSize != mEditorViewportSize)
		{
			ResizeViewport(newContentSize);
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const uint32 bufferedFrameId = gpu.GetBufferedFrameId();
		if (bufferedFrameId < mRenderedTextures.size())
		{
			ImGui::Texture(mRenderedTextures[bufferedFrameId]);
		}

		ImGui::End();
	}

	void FEditorLayer::ResizeViewport(const glm::uint2& newSize)
	{
		mEditorViewportSize = newSize;

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.WaitIdle();
		gpu.SetMainViewportSize(newSize);

		FWorld* world = gEngine->GetWorld();
		world->mRegistry.view<FCamera>().each([&](entt::entity entity, FCamera& camera)
		{
			camera.mAspectRatio = static_cast<float>(newSize.x) / static_cast<float>(newSize.y);
			world->mRegistry.get_or_emplace<FProjectionDirty>(entity);
		});

		// Destroy old textures
		for (THandle<FTexture> texture : mRenderedTextures)
		{
			gpu.DestroyTexture(texture);
		}
		mRenderedTextures.clear();

		// Create new ones
		const uint32 numBufferedFrames = gpu.GetNumBufferedFrames();
		mRenderedTextures.reserve(numBufferedFrames);
		for (uint32 frameId = 0; frameId < numBufferedFrames; ++frameId)
		{
			FTextureBuilder builder = {};
			builder
				.Init(vk::Format::eR8G8B8A8Unorm, ETextureType::Texture2D, ETextureFlags::Default)
				.SetSize(glm::uint3(newSize, 1))
				.SetName(FName(fmt::format("EditorViewport_{}", frameId)));

			mRenderedTextures.push_back(gpu.CreateTexture(builder));
		}
	}

	void FEditorLayer::HandleInputActionEvent(FActionEvent& event)
	{
		if (event.mName == kToggleFullscreenName && event.mbDown)
		{
			FWindow& window = entt::locator<FWindow>::value();
			window.SetFullscreen(!window.IsFullscreenEnabled());
			event.Handle();
		}
		else if (event.mName == kExitName && event.mbDown)
		{
			gEngine->RequestExit(EExitCode::Success);
			event.Handle();
		}
		else if (event.mName == kFrameCapture && event.mbDown)
		{
			entt::locator<IFrameDebuggerAPI>::value().CaptureFrame();
			event.Handle();
		}
	}

	void FEditorLayer::HandleCloseEvent(FCloseWindowEvent& event)
	{
		gEngine->RequestExit(EExitCode::Success);
		event.Handle();
	}
} // Turbo
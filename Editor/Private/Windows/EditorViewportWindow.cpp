#include "Windows/EditorViewportWindow.h"

#include "imgui.h"
#include "Core/Engine.h"
#include "EditorViewPort/EditorFreeCamera.h"
#include "Graphics/GPUDevice.h"
#include "Layers/ImGUILayer.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	void FEditorViewportWindow::Init()
	{
		FEditorFreeCameraUtils::Init();
	}

	void FEditorViewportWindow::Shutdown()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		for (THandle<FTexture> texture : mRenderedTextures)
		{
			gpu.DestroyTexture(texture);
		}
	}

	void FEditorViewportWindow::HandleEvent(FEventBase& event)
	{
		FEditorFreeCameraUtils::HandleEvent(event, bHasFocus);
	}

	void FEditorViewportWindow::Tick(float deltaTime)
	{
		FEditorFreeCameraUtils::Tick(deltaTime);
	}

	void FEditorViewportWindow::Draw()
	{
		ImGui::Begin("Viewport");
		bHasFocus = ImGui::IsWindowHovered();

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

	void FEditorViewportWindow::ResizeViewport(const glm::uint2& newSize)
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
} // Turbo
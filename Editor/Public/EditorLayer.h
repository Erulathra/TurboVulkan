#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Layers/Layer.h"

namespace Turbo
{
	class FEditorLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void BeginTick(double deltaTime) override;
		virtual void EndTick(double deltaTime) override;
		virtual bool ShouldTick() override;

		virtual void EndFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) override;
		virtual bool ShouldRender() override;

		virtual void OnEvent(FEventBase& event) override;

	public:
		virtual FName GetName() override;

	private:
		void DrawEditorViewport();
		void ResizeViewport(const glm::uint2& newSize);

	private:
		void HandleInputActionEvent(FActionEvent& event);
		void HandleCloseEvent(FCloseWindowEvent& event);

	private:
		std::vector<THandle<FTexture>> mRenderedTextures;
		std::vector<vk::DescriptorSet> mVkRenderedTexturesDescriptorSets;

		glm::uint2 mEditorViewportSize = glm::uint2(0);
	};
} // Turbo

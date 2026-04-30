#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Layers/Layer.h"
#include "Windows/EditorViewportWindow.h"
#include "Windows/SceneOutlinerWindow.h"

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
		void HandleInputActionEvent(FActionEvent& event);
		void HandleCloseEvent(FCloseWindowEvent& event);

	public:
		TSharedPtr<FEditorViewportWindow> mViewportWindow;
		TSharedPtr<FSceneOutlinerWindow> mOutlinerWindow;
	};
} // Turbo

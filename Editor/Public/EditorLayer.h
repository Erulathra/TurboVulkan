#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Layers/Layer.h"
#include "Windows/EditorViewportWindow.h"
#include "Windows/SceneOutlinerWindow.h"

namespace Turbo
{
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged, entt::entity);

	class FEditorLayer : public ILayer
	{
		FOnSelectionChanged OnSelectionChanged;

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

	public:
		void SetSelection(entt::entity entity);
		[[nodiscard]] entt::entity GetSelection() const { return mSelection; }

	private:
		entt::entity mSelection = entt::null;

	public:
		TSharedPtr<FEditorViewportWindow> mViewportWindow;
		TSharedPtr<FSceneOutlinerWindow> mOutlinerWindow;


	private:
		void HandleInputActionEvent(FActionEvent& event);
		void HandleCloseEvent(FCloseWindowEvent& event);

	};

	template<>
	inline FName GetStaticLayerName<FEditorLayer>()
	{
		static FName layerName = FName("EditorLayer");
		return layerName;
	}
} // Turbo

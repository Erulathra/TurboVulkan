#pragma once

#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Layers/Layer.h"

namespace Turbo
{
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged, entt::entity);

	class FEditorViewportWindow;
	class FPropertyEditorWindow;
	class FSceneOutlinerWindow;

	class FEditorSelection
	{
	public:
		FOnSelectionChanged OnSelectionChanged;

	public:
		void SetSelection(entt::entity entity);
		[[nodiscard]] entt::entity GetSelection() const { return mSelection; }

	private:
		entt::entity mSelection = entt::null;
	};

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

	public:
		TSharedPtr<FEditorViewportWindow> mViewportWindow;
		TSharedPtr<FSceneOutlinerWindow> mOutlinerWindow;
		TSharedPtr<FPropertyEditorWindow> mPropertyEditor;


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

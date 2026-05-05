#include "imgui.h"
#include "RenderingTestLayer.h"

#include "EditorLayer.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Debug/IConsoleManager.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "Graphics/ResourceBuilders.h"
#include "World/World.h"

namespace Turbo
{

	void FRenderingTestLayer::Start()
	{
		FWorld* world = gEngine->GetWorld();
		world->OpenLevel(FName("Content/External/main_sponza/SponzaCompressed.gltf"));
	}

	void FRenderingTestLayer::Shutdown()
	{
	}

	void FRenderingTestLayer::ShowImGuiWindow()
	{
		ImGui::Begin("Rendering test");

		constexpr uint32 frameTimeHistorySize = 256;
		static float frameTimeHistory[frameTimeHistorySize];

		frameTimeHistory[FCoreTimer::TickIndex() % frameTimeHistorySize] = static_cast<float>(FCoreTimer::DeltaTime());

		float AvgFrameTime = 0;
		for (float frameTime : frameTimeHistory)
		{
			AvgFrameTime += frameTime;
		}
		AvgFrameTime /= frameTimeHistorySize;

		ImGui::Text("Frame time: %f, FPS: %f", AvgFrameTime, 1.f / AvgFrameTime);

		ImGui::PlotHistogram(
			"Frame time graph",
			frameTimeHistory,
			frameTimeHistorySize,
			0,
			nullptr,
			0.f
		);

		FEditorLayer* editorLayer = entt::locator<FLayersStack>::value().GetLayerChecked<FEditorLayer>();
		entt::entity selection = editorLayer->GetSelection();

		std::string selectionString = "None";

		entt::registry& registry = gEngine->GetWorld()->mRegistry;
		const FEntityName* selectionName = registry.try_get<FEntityName>(selection);
		if (selectionName)
		{
			selectionString = selectionName->mName.ToString();
		}
		else if (registry.valid(selection))
		{
			selectionString = fmt::format("Entity: {}", static_cast<uint32>(selection));
		}

		ImGui::TextFmt("Selected entities: {}", selectionString);

		ImGui::End();
	}

	void FRenderingTestLayer::BeginTick(double deltaTime)
	{
		ShowImGuiWindow();
	}

	template<>
	FName GetStaticLayerName<FRenderingTestLayer>()
	{
		static FName layerName = FName("RenderingTestLayer");
		return layerName;
	}

	FName FRenderingTestLayer::GetName()
	{
		return GetStaticLayerName<FRenderingTestLayer>();
	}
}

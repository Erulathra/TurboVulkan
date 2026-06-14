#include "imgui.h"
#include "RenderingTestLayer.h"

#include "EditorLayer.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Debug/IConsoleManager.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "Graphics/Shaders/ToneMapperPostProcess.h"
#include "World/ShadingComponents.h"
#include "World/World.h"

namespace Turbo
{

	void FRenderingTestLayer::Start()
	{
		FWorld* world = gEngine->GetWorld();
		world->OpenLevel(FName("Content/External/main_sponza/compressed/NewSponza_Main_glTF_003.gltf"));
		// world->OpenLevel(FName("Content/Scenes/LV_GammaTest.gltf"));
		// world->OpenLevel(FName("/home/erulathra/Documents/glTF-Sample-Assets/Models/NormalTangentMirrorTest/glTF/compressed/NormalTangentMirrorTest.gltf"));

		auto& registry = world->mRegistry;
		entt::entity ppSettingsEntity = registry.create();
		registry.emplace<FEntityLabel>(ppSettingsEntity, FName("PostProcessSettings"));
		registry.emplace<FWorldRoot>(ppSettingsEntity);
		registry.emplace<ToneMapperPostProcess::FComponent>(ppSettingsEntity);

#if 0
		// Normal testing
		const entt::entity lightEntt = registry.create();
		registry.emplace<FEntityLabel>(lightEntt, FName("PointLight"));
		FTransform& transform = registry.emplace<FTransform>(lightEntt);
		transform.mPosition = glm::float3(0., 0., -2.f);
		registry.emplace<FRelationship>(lightEntt);
		FLightComponent& lightComponent = registry.emplace<FLightComponent>(lightEntt);
		lightComponent.mIntensity = 10.0f;
#endif
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

		entt::entity selection = entt::locator<FEditorSelection>::value().GetSelection();

		std::string selectionString = "None";

		entt::registry& registry = gEngine->GetWorld()->mRegistry;
		const FEntityLabel* selectionName = registry.try_get<FEntityLabel>(selection);
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

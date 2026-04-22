#include "imgui.h"
#include "RenderingTestLayer.h"

#include "Assets/EngineResources.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Debug/IConsoleManager.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "glm/gtx/string_cast.hpp"
#include "Graphics/ResourceBuilders.h"
#include "World/Camera.h"
#include "World/CommonEntities.h"
#include "World/MeshComponent.h"
#include "World/World.h"

using namespace Turbo;

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

	const entt::registry& registry = gEngine->GetWorld()->mRegistry;
	const auto mainCameraView = registry.view<FWorldTransform const, FCamera const, FMainViewport const>();

	for (entt::entity entity : mainCameraView)
	{
		const FWorldTransform& transform = mainCameraView.get<FWorldTransform>(entity);
		ImGui::TextFmt("Camera position: {}", TransformUtils::GetPosition(transform));
		ImGui::TextFmt("Camera front: {}", TransformUtils::GetForward(transform));
		ImGui::TextFmt("Camera right: {}", TransformUtils::GetRight(transform));
		ImGui::TextFmt("Camera up: {}", TransformUtils::GetUp(transform));
	}

	ImGui::End();
}

void FRenderingTestLayer::BeginTick(double deltaTime)
{
	ShowImGuiWindow();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}


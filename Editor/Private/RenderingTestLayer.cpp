#include "imgui.h"
#include "RenderingTestLayer.h"

#include "Assets/EngineResources.h"
#include "Assets/MaterialManager.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/MeshComponent.h"
#include "World/World.h"

using namespace Turbo;

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	TRACE_ZONE_SCOPED()

	FWorld* world = gEngine->GetWorld();
	world->OpenLevel(FName("Content/External/main_sponza/SponzaCompressed.gltf"));

	entt::entity errorEntity = world->mRegistry.create();
	world->mRegistry.emplace<FTransform>(errorEntity);
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


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

	// FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
	// THandle<FMaterial> materialHandle = materialManager.GetMaterial(EngineMaterials::kTriangleTest);

	entt::entity errorEntity = world->mRegistry.create();
	world->mRegistry.emplace<FTransform>(errorEntity);
	FMeshComponent& meshComponent = world->mRegistry.emplace<FMeshComponent>(errorEntity);
	// meshComponent.mMaterial = materialHandle;
}

void FRenderingTestLayer::Shutdown()
{
}

void FRenderingTestLayer::ShowImGuiWindow()
{
	ImGui::Begin("Rendering test");
	ImGui::Text("Frame time: %f, FPS: %f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());

	constexpr uint32 frameTimeHistorySize = 256;
	static float frameTimeHistory[frameTimeHistorySize];

	frameTimeHistory[FCoreTimer::TickIndex() % frameTimeHistorySize] = static_cast<float>(FCoreTimer::DeltaTime());

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


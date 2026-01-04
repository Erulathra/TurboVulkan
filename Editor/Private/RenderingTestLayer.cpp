#include "imgui.h"
#include "RenderingTestLayer.h"

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

#if 1
	FWorld* world = gEngine->GetWorld();
	world->OpenLevel(FName("Content/External/main_sponza/SponzaCompressed.gltf"));
#else
	FAssetManager& assetManager = entt::locator<FAssetManager>::value();

	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
	FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("MeshTestMaterial.slang");
	THandle<FMaterial> materialHandle = materialManager.LoadMaterial<void, void>(pipelineBuilder, 1);
	THandle<FMaterial::Instance> materialInstanceHandle = materialManager.CreateMaterialInstance(materialHandle);
	FWorld& world = *gEngine->GetWorld();

	entt::entity nodeEntity = world.mRegistry.create();
	world.mRegistry.emplace<FSpawnedByLevelTag>(nodeEntity);
	world.mRegistry.emplace<FRelationship>(nodeEntity);
	world.mRegistry.emplace<FTransform>(nodeEntity);

	FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(nodeEntity);
	meshComponent.mMaterial = materialHandle;
	meshComponent.mMaterialInstance = materialInstanceHandle;

	meshComponent.mMesh = assetManager.LoadMesh(FName("Content/Meshes/SM_BlenderMonkey.glb"));
#endif
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


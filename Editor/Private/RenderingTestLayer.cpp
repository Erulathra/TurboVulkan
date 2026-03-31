#include "imgui.h"
#include "RenderingTestLayer.h"

#include "Assets/EngineResources.h"
#include "Assets/MaterialManager.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Extensions/ImGui/ImGuiExtensions.h"
#include "glm/gtx/string_cast.hpp"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/Camera.h"
#include "World/CommonEntities.h"
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
	// world->OpenLevel(FName("Content/Meshes/SM_Cube.glb"));

#if 0
	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	THandle<FMesh> cube = assetManager.LoadMesh(FName("Content/Meshes/SM_Cube.glb"));

	const uint32 numCubesSqrt = 3;
	for (uint32 cubeX = 0; cubeX < numCubesSqrt; ++cubeX)
	{
		for (uint32 cubeY = 0; cubeY < numCubesSqrt; ++cubeY)
		{
			for (uint32 cubeZ = 2; cubeZ < numCubesSqrt; ++cubeZ)
			{
				entt::entity meshEntity = CommonEntities::SpawnStaticMeshEntity(world->mRegistry);
				FMeshComponent& meshComponent = world->mRegistry.get<FMeshComponent>(meshEntity);
				meshComponent.mMesh = cube;
				meshComponent.mMaterial = EngineMaterials::GetPlaceholderMaterial();

				const glm::float3 offset = glm::vec3((numCubesSqrt - 1) * 0.5f);
				world->mRegistry.replace<FTransform>(meshEntity) =
					FTransform((glm::float3(cubeX, cubeY, cubeZ) - offset) * 16.f);
			}
		}
	}
#endif

	// for (auto&& [entity, camera, transform] : world->mRegistry.view<FCamera, FTransform>().each())
	// {
	// 	transform.mPosition = glm::vec3(0.0f, 0.0f, -10.0f);
	// }
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


#include "imgui.h"
#include "RenderingTestLayer.h"
#include "Assets/AssetManager.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/Math/Vector.h"
#include "glm/gtx/compatibility.hpp"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/Camera.h"
#include "World/MeshComponent.h"
#include "World/World.h"

using namespace Turbo;

struct FPushConstants
{
	glm::float4x4 objToProj;

	FDeviceAddress positionBuffer;
	FDeviceAddress normalBuffer;
	FDeviceAddress uvBuffer;
	uint32 textureId;
	uint32 samplerId;
};

struct FRotateComponent
{
	float speed;
};

entt::entity CreatePivot(FWorld& world, entt::entity parent, float offset, float rotationSpeed)
{
	entt::registry& registry = world.mRegistry;

	// create entity
	const entt::entity entity = registry.create();
	registry.emplace<FRelationship>(entity);
	if (parent != entt::null)
	{
		world.AddChild(parent, entity);
	}

	// initialize components
	FTransform& transform = registry.emplace<FTransform>(entity);
	transform.mPosition.x = offset;
	FRotateComponent& rotateComponent = registry.emplace<FRotateComponent>(entity);
	rotateComponent.speed = rotationSpeed;

	return entity;
}

entt::entity CreateCelestialBody(FWorld& world, entt::entity parent, float offset, float rotationSpeed, THandle<FMesh> meshHandle, THandle<FMaterial::Instance> matInstanceHandle)
{
	entt::entity entity = CreatePivot(world, parent, offset, rotationSpeed);

	FMaterial::Instance* matInstance = entt::locator<FMaterialManager>::value().AccessInstance(matInstanceHandle);

	FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(entity);
	meshComponent.mMesh = meshHandle;
	meshComponent.mMaterial = matInstance->material;
	meshComponent.mMaterialInstance = matInstanceHandle;

    return entity;
}

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	TRACE_ZONE_SCOPED()

	FWorld& world = *gEngine->GetWorld();

	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

	// THandle<FMesh> meshHandle = assetManager.LoadMesh("Content/Meshes/SM_IcoPlanet.glb").front();
	// THandle<FMesh> meshHandle = assetManager.LoadMesh("Content/Meshes/SM_Cube.glb").front();
	THandle<FMesh> meshHandle = assetManager.LoadMesh("Content/Meshes/SM_BlenderMonkey.glb").front();
	FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("BaseMaterial.slang");
	THandle<FMaterial> materialHandle = materialManager.LoadMaterial(pipelineBuilder, 0, 1);
	THandle<FMaterial::Instance> instanceHandle = materialManager.CreateMaterialInstance(materialHandle);

	mSunEntity = CreateCelestialBody(world, entt::null, 0.f, 0.f, meshHandle, instanceHandle);

	for (uint32 planetId = 1; planetId < 3; ++planetId)
	{
		constexpr float offset = 3.f;
		const entt::entity pivot = CreatePivot(world, entt::null, 0.f, 1.f / static_cast<float>(planetId));
		const entt::entity planet = CreateCelestialBody(world, pivot, offset * static_cast<float>(planetId), 1.f, meshHandle, instanceHandle);
	}
}

void FRenderingTestLayer::Shutdown()
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;

	entt::dense_set<THandle<FMesh>> usedMeshes;
	entt::dense_set<THandle<FMaterial>> usedMaterials;

	auto view = registry.view<FMeshComponent>();
	for (entt::entity entity : view)
	{
		const FMeshComponent& meshComponent = view.get<FMeshComponent>(entity);
		usedMeshes.emplace(meshComponent.mMesh);
		usedMaterials.emplace(meshComponent.mMaterial);
	}

	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	for (THandle<FMesh> mesh : usedMeshes)
	{
		assetManager.UnloadMesh({mesh});
	}

	FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
	for (THandle<FMaterial> material : usedMaterials)
	{
		materialManager.DestroyMaterial(material);
	}
}

void FRenderingTestLayer::ShowImGuiWindow()
{
	FWorld* world = gEngine->GetWorld();
	FTransform transform = world->mRegistry.get<FTransform>(mSunEntity);

	ImGui::Begin("Rendering test");
	ImGui::Text("Frame time: %f, FPS: %f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());
	ImGui::DragFloat3("Sun Pos", glm::value_ptr(transform.mPosition));
	ImGui::End();

	world->mRegistry.replace<FTransform>(mSunEntity, transform);
}

void FRenderingTestLayer::BeginTick(double deltaTime)
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;
	auto rotationView = registry.view<FTransform, FRotateComponent>();

	for (const entt::entity entity : rotationView)
	{
		FTransform newTransform = rotationView.get<FTransform>(entity);
		const FRotateComponent& rotateComponent = rotationView.get<FRotateComponent>(entity);

		newTransform.mRotation = glm::quat(EFloat3::Up * rotateComponent.speed * static_cast<float>(deltaTime)) * newTransform.mRotation;

		registry.replace<FTransform>(entity, newTransform);
	}

	ShowImGuiWindow();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}
